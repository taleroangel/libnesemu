/* -- Standard Libraries -- */

#include "nesemu/ppu/palette.h"
#include "nesemu/ppu/ppu.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

/* NES emulation library */
#include <nesemu/nesemu.h>

/* Other libraries */
#include <raylib.h>

/* Screen size multiplier */
#define SCALING_FACTOR 3

/** Flag for the main event loop */
static volatile bool g_main_event_loop = true;

void sigint_handler(__attribute__((unused)) int _)
{
	g_main_event_loop = false;
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

	/* Initialize Raylib */
	InitWindow(NESEMU_WIDTH * SCALING_FACTOR,
            NESEMU_HEIGHT * SCALING_FACTOR,
            argv[1]);

	Texture2D texture = LoadTextureFromImage((Image){
		.data = NULL,
		.width = NESEMU_WIDTH,
		.height = NESEMU_HEIGHT,
		.mipmaps = 1,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
	});

	printf("Emulator setup completed, running.\n");
	fflush(stdout);

	printf("Press <Ctrl-C> to kill the emulator.\n");
	signal(SIGINT, sigint_handler);

	/* Main event loop */
	while (g_main_event_loop && !WindowShouldClose()) {
		nesemu_return_t err = NESEMU_RETURN_SUCCESS;

		// Let the PPU render the frame
		int ppu_cycles = 0;
		err = nes_ppu_render(&ppu, &framebuffer, &mem, &vim, &ppu_cycles);
		if (err != NESEMU_RETURN_SUCCESS) {
			fprintf(stderr, "nesemu: PPU failed to execute");
			break; /* Exit main loop */
		}

		// Transform texture from XRGB to RGBA
		for (size_t i = 0; i < NESEMU_PPU_BUFFER_SIZE; i++) {
            // Byte order reversal
            uint32_t color = framebuffer[i];
            uint8_t r = (color >> 16) & 0xFF,
                    g = (color >> 8) & 0xFF,
                    b = color & 0xFF;
			framebuffer[i] = r | (g << 8) | (b << 16) | (0xFFL << 24);
		}
		UpdateTexture(texture, (const void *)framebuffer);

        // Draw frame
		BeginDrawing();
		DrawTexturePro(texture,
			       (Rectangle){ 0, 0, NESEMU_WIDTH, NESEMU_HEIGHT },
			       (Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight() },
			       (Vector2){ 0, 0 }, .0f, WHITE);
		EndDrawing();

		// Catch up with the ppu
		int cpu_cycles = 0;
		while ((3 * cpu_cycles) < ppu_cycles) {
			// Execute next instruction
			int instruction_cpu_cycles = 0;
			err = nes_cpu_next(&cpu, &mem, &instruction_cpu_cycles);
			if (err != NESEMU_RETURN_SUCCESS) {
				fprintf(stderr, "nesemu: cpu execution error (%d)", (int)err);
				g_main_event_loop = false; /* Exit main event loop */
				break;
			}
			cpu_cycles += instruction_cpu_cycles;
		}

		// CPU was stopped
		if (cpu.stop) {
			g_main_event_loop = false;
		}
	}

	CloseWindow();
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
