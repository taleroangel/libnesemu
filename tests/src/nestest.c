/**
 * Load Kevin Horton's "nestest" ROM to test CPU instructions
 *
 * Reference:
 * https://www.qmtpro.com/~nes/misc/nestest.txt
 */

#include "nesemu/util/error.h"
#include "nesemu/memory/main.h"
#include "nesemu/cpu/cpu.h"
#include "nesemu/cartridge/cartridge.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define CARTRIDGE_NAME "nestest.nes"
#define PATH_MAX 4096

/**
 * Initial $pc address for the headless CPU test
 *
 * Check details at:
 * https://www.qmtpro.com/~nes/misc/nestest.txt
 */
#define START_PC 0xC000

/**
 * Read cartridge from FS into cdata
 *
 * @note cdata contents are allocated with 'malloc'
 */
int read_cartridge(uint8_t **cdata, size_t *len);

/**
 * Run the test with already initialized hardware
 */
int run(struct nes_main_memory_t *mem, struct nes_cpu_t *cpu);

/* Entry point */

int main(void)
{
	// Print test information

	// Get working directory
	char workdir[PATH_MAX];
	(void)getcwd(workdir, PATH_MAX);
	printf("Current Working Directory = '%s'\n", workdir);

	// Placeholder for the error
	nesemu_return_t err = NESEMU_RETURN_SUCCESS;

    /* -- Load cartridge -- */

    // Create cartridge
    struct nes_cartridge_t cartridge;

	// Load cdata
	size_t clen = 0;
	uint8_t *cdata = NULL;
	if (read_cartridge(&cdata, &clen) != EXIT_SUCCESS) {
		perror("nesemu cartridge read failed with system error");
		return EXIT_FAILURE;
	}

	// Load cartridge
	if (nes_cartridge_read_ines(&cartridge, cdata, clen) !=
	    NESEMU_RETURN_SUCCESS) {
		// Deallocate file data
		free(cdata);
		cdata = NULL;

		// Print error message
		fprintf(stderr,
			"nesemu cartridge initialization failed with code (0x%x)\n",
			err);
		return EXIT_FAILURE;
	}

	// Deallocate file data
	free(cdata);
	cdata = NULL;

	printf("Successful NESEMU cartridge initialization\n");

	/* -- Memory initialization -- */

	// Initialize memory
	struct nes_main_memory_t mem;
	err = nes_mem_init(&mem, &cartridge);
	if (err != NESEMU_RETURN_SUCCESS) {
		fprintf(stderr,
			"nesemu memory initialization failed with code (0x%x)\n",
			err);
		return EXIT_FAILURE;
	}

	printf("Successful NESEMU memory initialization\n");	

	/* -- CPU Initialization -- */
	struct nes_cpu_t cpu;
	err = nes_cpu_init(&cpu, &mem);
	if (err != NESEMU_RETURN_SUCCESS) {
		fprintf(stderr,
			"nesemu cpu initialization failed with code (0x%x)\n",
			err);
		return EXIT_FAILURE;
	}

	printf("Successful NESEMU cpu initialization\n");

	/* -- Run -- */
	if (run(&mem, &cpu) != EXIT_SUCCESS) {
		return EXIT_FAILURE;
	}

	printf("Successful NESEMU run!\n");

	return EXIT_SUCCESS;
}

int run(struct nes_main_memory_t *mem, struct nes_cpu_t *cpu)
{
	nesemu_return_t err = NESEMU_RETURN_SUCCESS;
	long int tcycles = 0, tinstructions = 0;

	// Set program counter
	cpu->pc = START_PC;

	// Infinite execution loop
	while (!cpu->stop) {
		// Execute next instruction
		int cpu_cycles = 0;
		err = nes_cpu_next(cpu, mem, &cpu_cycles);
		if (err != NESEMU_RETURN_SUCCESS) {
			break;
		}

		// Add to the total execution count
		tcycles += cpu_cycles;
		tinstructions++;
	}

	printf("CPU execution completed (Cycles=%ld, Instructions=%ld, $pc=0x%04x, $brk=0x%02x)\n",
	       tcycles, tinstructions, cpu->pc, cpu->brk);

	// Return error
	if (err != NESEMU_RETURN_SUCCESS) {
		printf("nesemu CPU execution failed with error code (0x%x)\n",
		       err);
		return EXIT_FAILURE;
	}

	// Read error codes
	uint8_t err0 = 0, err1 = 0;
	if ((err = nes_mem_r8(mem, 0x0002, &err0)) != NESEMU_RETURN_SUCCESS) {
		printf("failed to read memory at $0002");
		return EXIT_FAILURE;
	}

	if ((err = nes_mem_r8(mem, 0x0003, &err1)) != NESEMU_RETURN_SUCCESS) {
		printf("failed to read memory at $0003");
		return EXIT_FAILURE;
	}

	// nestest Error
	if (cpu->brk != 0x00 || err0 != 0 || err1 != 0) {
		printf("nestest failure ($brk=0x%04x, $02=%02x, $03=%02x)\n",
		       cpu->brk, err0, err1);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int read_cartridge(uint8_t **cdata, size_t *len)
{
	// Read file
	FILE *f = fopen(CARTRIDGE_NAME, "r");
	if (f == NULL) {
		return errno;
	}

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

	// Close file
	if (fclose(f) != 0) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
