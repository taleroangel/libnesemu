/**
 * Code to load iNES cartridge format
 *
 * Reference:
 * https://www.emulationonline.com/systems/nes/ines-loading/
 */

#ifndef __NESEMU_CARTRIDGE_H__
#define __NESEMU_CARTRIDGE_H__

#include "types/common.h"
#include "types/nrom.h"

#include "nesemu/util/compat.h"
#include "nesemu/util/error.h"

#include <stddef.h>
#include <stdint.h>

/**
 * iNES cartridge header size
 */
#define NESEMU_CARTRIDGE_INES_HEADER_SIZE 16

/**
 * iNES cartridge PRGROM size (in chunks) bit index
 */
#define NESEMU_CARTRIDGE_INES_HEADER_PRGROM_SIZE_INDEX 4

/**
 * iNES cartridge PRGROM chunk size
 */
#define NESEMU_CARTRIDGE_INES_HEADER_PRGROM_CHUNK_SIZE 0x4000 /* 16KiB */

/**
 * iNES cartridge CHRROM size (in chunks) bit index
 */
#define NESEMU_CARTRIDGE_INES_HEADER_CHRROM_SIZE_INDEX 5

/**
 * iNES cartridge CHRROM chunk size
 */
#define NESEMU_CARTRIDGE_INES_HEADER_CHRROM_CHUNK_SIZE 0x2000 /* 8KiB */

/**
 * Index for the byte defining the mapper type
 */
#define NESEMU_CARTRIDGE_INES_HEADER_MAPPER_TYPE_INDEX 6

/**
 * Variant type for iNES cartridge mappers
 */
union nes_ines_mapper_t {
	struct nes_ines_nrom_cartridge_t nrom; /**< NROM (0) */

	/**
     * Just an empty field :)
     * Because this is a `union` type. Grabbing a reference to this value
     * (or any other value) will be the same as having a generic reference
     * to any of the union's members;
     *
     * Do not use this field directly. Let the macros handle the dark arts.
     */
	_NESEMU_EMPTY_TYPE __self;
};

/**
 * iNES cartridge mappers variant discriminator
 */
enum nes_ines_mapper_variant {
	NESEMU_INES_MAPPER_UNSUPPORTED = -1,
	NESEMU_INES_MAPPER_NROM = 0,
};

/**
 * iNES cartridge with appropiate mapper callbacks
 */
struct nes_cartridge_t {
	enum nes_ines_mapper_variant type; /**< Variant discriminator */
	union nes_ines_mapper_t mapper; /**< Variant type */

	/* -- Mapper Callbacks -- */
	nes_cartridge_read_t cpu_reader; /**< Method to read program data from cartridge */
	nes_cartridge_write_t cpu_writer; /**< Method to write program data into cartridge */
	nes_cartridge_loader_t cpu_loader; /**< Method to load program data into the cartridge struct */

	nes_cartridge_read_t ppu_reader; /**< Method to read from cartridge's chrrom/chrram */
	nes_cartridge_write_t ppu_writer; /**< Method to write into cartridge's chrram */
	nes_cartridge_loader_t ppu_loader; /**< Method to load video data into the cartridge struct */
};

/**
 * Get a generic reference to the mapper internal structure.
 * Use this as first parameter of the mapper callbacks
 *
 * @param c (struct nes_cartridge_t) cartridge
 */
#define NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(c) \
	(nesemu_mapper_generic_ref_t) & c.mapper.__self

/**
 * Load iNES cartridge into memory
 *
 * @note Do this before the CPU is initialized
 */
nesemu_error_t nes_read_ines(struct nes_cartridge_t *cartridge,
			     uint8_t *data,
			     size_t len);

#endif
