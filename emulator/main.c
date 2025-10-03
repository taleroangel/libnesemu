/* -- Standard Libraries -- */
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <nesemu/util/error.h>
#include <nesemu/cartridge/cartridge.h>
#include <nesemu/memory/main.h>
#include <nesemu/cpu/cpu.h>

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
        printf("Reading cartridge from: %s\n.", cartridge_path);

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
        if ((err = nes_cartridge_read_ines(&cartridge, cartridge_data, cartridge_data_size)) != NESEMU_RETURN_SUCCESS) {
            fprintf(stderr, "Failed to read cartridge, code = %04X", err);
            return EXIT_FAILURE;
        }
        free(cartridge_data);
    }

    /* Initialize memory */
    nes_mem_t mem;
    if ((err = nes_mem_init(&mem, &cartridge)) != NESEMU_RETURN_SUCCESS) {
        fprintf(stderr, "Failed to initialize memory, code = %04X", err);
        return EXIT_FAILURE;
    }

    /* Initialize CPU */
    nes_cpu_t cpu;
    if ((err = nes_cpu_init(&cpu, &mem)) != NESEMU_RETURN_SUCCESS) {
        fprintf(stderr, "Failed to initialize memory, code = %04X", err);
        return EXIT_FAILURE;
    }

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
