/* -- Standard Libraries -- */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <nesemu/nesemu.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pixels.h>

/* Linux (and maybe Windows?) will use Vulkan, macos will use Metal */
#ifdef __APPLE__
#define SDL_WINDOW_GRAPHICS_API (SDL_WINDOW_METAL)
#else
#define SDL_WINDOW_GRAPHICS_API (SDL_WINDOW_VULKAN)
#endif

/** Flag for the main event loop */
static volatile bool main_event_loop = true;

void sigint_handler(int _)
{
	main_event_loop = false;
}

/**
 * Read cartridge raw data bytes from file
 */
int read_cartridge(FILE *f, uint8_t **cdata, size_t *len);

int main(int argc, char *argv[])
{
	nesemu_return_t err = NESEMU_RETURN_SUCCESS;

	/* Check arguments */
	if (argc < 2) {
		fprintf(stderr, "Bad arguments!, missing cartridge path.\n");
		return EXIT_FAILURE;
	}

	/* Read cartridge data */
	nes_cartridge_t cartridge;

	{ /* Read cartridge from file into structure */

		/* Get path to the cartridge */
		char *cartridge_path = argv[1];
		printf("Reading cartridge from: %s\n", cartridge_path);

		/* Read cartridge */
		uint8_t *cartridge_data;
		size_t cartridge_data_size;
		FILE *cartridge_file = fopen(cartridge_path, "rb");
		if (read_cartridge(cartridge_file, &cartridge_data,
				   &cartridge_data_size) != EXIT_SUCCESS) {
			perror("Failed to read cartridge data");
		}
		fclose(cartridge_file);

		/* Allocate into cartridge structure */
		if ((err = nes_cartridge_read_ines(&cartridge, cartridge_data,
						   cartridge_data_size)) !=
		    NESEMU_RETURN_SUCCESS) {
			fprintf(stderr, "Failed to read cartridge, code = %04X",
				err);
			return EXIT_FAILURE;
		}
		free(cartridge_data);
	}

	/* Initialize memory */
	nes_mem_main_t mem;
	if ((err = nes_mem_init(&mem, &cartridge)) != NESEMU_RETURN_SUCCESS) {
		fprintf(stderr, "Failed to initialize main memory, code = %04X",
			err);
		return EXIT_FAILURE;
	}

	nes_mem_video_t vim;
	if ((err = nes_vram_init(&vim, &cartridge)) != NESEMU_RETURN_SUCCESS) {
		fprintf(stderr,
			"Failed to initialize video memory, code = %04X", err);
		return EXIT_FAILURE;
	}

	/* Initialize CPU */
	nes_cpu_t cpu;
	if ((err = nes_cpu_init(&cpu, &mem)) != NESEMU_RETURN_SUCCESS) {
		fprintf(stderr, "Failed to initialize cpu, code = %04X", err);
		return EXIT_FAILURE;
	}

	/* Initialize PPU */
	nes_ppu_system_palette_t palette = NESEMU_PALETTE_STANDARD;
	nes_ppu_t ppu;
	if ((err = nes_ppu_init(&ppu, &palette, &mem)) !=
	    NESEMU_RETURN_SUCCESS) {
		fprintf(stderr, "Failed to initialize cpu, code = %04X", err);
		return EXIT_FAILURE;
	}

	/* Create the display framebuffer */
	nes_display_t framebuffer;

	printf("Press <Ctrl-C> to kill the emulator.\n");
	signal(SIGINT, sigint_handler);

	/* Initialize SDL */
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		fprintf(stderr, "Failed to initialize SDL!: %s",
			SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_Window *window = SDL_CreateWindow(
		"nesemu", NESEMU_WIDTH, NESEMU_HEIGHT, SDL_WINDOW_GRAPHICS_API);

	if (!window) {
		fprintf(stderr, "Failed to create window: %s", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
	if (!renderer) {
		fprintf(stderr, "Unable to initialize renderer: %s",
			SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	// Create target texture
	SDL_Texture *texture = SDL_CreateTexture(renderer,
						 SDL_PIXELFORMAT_XRGB8888,
						 SDL_TEXTUREACCESS_STREAMING,
						 NESEMU_WIDTH, NESEMU_HEIGHT);
	if (!texture) {
		fprintf(stderr, "Unable to create texture: %s", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	printf("Emulator setup completed, running.\n");
	fflush(stdout);

	while (main_event_loop) {
		/* Polling event */
		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_EVENT_QUIT) {
				main_event_loop = false;
			}
		}

		nesemu_return_t err = NESEMU_RETURN_SUCCESS;

		// PPU must render
		int ppu_cycles = 0;
		err = nes_ppu_render(&ppu, &framebuffer, &mem, &vim,
				     &ppu_cycles);
		if (err != NESEMU_RETURN_SUCCESS) {
			fprintf(stderr, "nesemu: PPU failed to execute");
			main_event_loop = false;
			break;
		}

		int tcpu_cycles = 0;

		// Upload texture
		void *dst_pixels = NULL;
		int dst_pitch = 0;
		if (!SDL_LockTexture(texture, NULL, &dst_pixels, &dst_pitch)) {
			fprintf(stderr, "Failed to lock texture: %s",
				SDL_GetError());
			main_event_loop = false;
			break;
		}

		const uint8_t *src = (const uint8_t *)framebuffer;
		uint8_t *dst = (uint8_t *)dst_pixels;

		const int src_pitch = NESEMU_WIDTH * sizeof(uint32_t);

		for (int y = 0; y < NESEMU_HEIGHT; ++y) {
			memcpy(dst + y * dst_pitch, src + y * src_pitch,
			       src_pitch);
		}

		SDL_UnlockTexture(texture);

		// Render frame
		SDL_RenderClear(renderer);
		SDL_RenderTexture(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		// Catch up with the ppu
		while ((3 * tcpu_cycles) < ppu_cycles) {
			// Execute next instruction
			int cpu_cycles = 0;
			err = nes_cpu_next(&cpu, &mem, &cpu_cycles);
			if (err != NESEMU_RETURN_SUCCESS) {
				fprintf(stderr,
					"nesemu: cpu execution error (%d)",
					(int)err);
				main_event_loop = false;
				break;
			}
			tcpu_cycles += cpu_cycles;
		}

		// CPU was stopped
		if (cpu.stop) {
			main_event_loop = false;
		}
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}

int read_cartridge(FILE *f, uint8_t **cdata, size_t *len)
{
	// Seek to end
	if (fseek(f, 0L, SEEK_END) != 0) {
		return EXIT_FAILURE;
	}

	// Get the size of the file
	*len = ftell(f);

	// Seek to start
	if (fseek(f, 0L, SEEK_SET) != 0) {
		return EXIT_FAILURE;
	}

	// Allocate data
	*cdata = (uint8_t *)malloc(*len);

	// Copy data from file
	size_t rb = fread(*cdata, 1L, *len, f);
	if (rb != *len) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
