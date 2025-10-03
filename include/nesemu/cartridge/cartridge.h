/**
 * Code to load iNES cartridge format
 *
 * References:
 * https://www.emulationonline.com/systems/nes/ines-loading/
 * https://www.nesdev.org/wiki/INES
 */

#ifndef __NESEMU_CARTRIDGE_H__
#define __NESEMU_CARTRIDGE_H__

#include "types/common.h"
#include "types/nrom.h"

#include "nesemu/util/compat.h"
#include "nesemu/util/error.h"

#include <stdbool.h>
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
 * Index for the FLAGS 6 byte
 */
#define NESEMU_CARTRIDGE_INES_HEADER_FLAGS_6_INDEX 6

/**
 * Index for the FLAGS 7 byte
 */
#define NESEMU_CARTRIDGE_INES_HEADER_FLAGS_7_INDEX 7

/**
 * Variant type for iNES cartridge mappers
 */
union nes_ines_mapper_t {
	struct nes_ines_nrom_cartridge_t nrom; /**< NROM (0) */

	/**
     * Just an empty field for type punning :)
     * Because this is a `union` type. Grabbing a reference to this value
     * (or any other value) will be the same as having a generic reference
     * to any of the union's members (type punning)
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
 * Flags 6 from the iNES header bit masks
 *
 * 76543210
 * ||||||||
 * |||||||+- Nametable arrangement: 0: vertical arrangement ("horizontal mirrored") (CIRAM A10 = PPU A11)
 * |||||||                          1: horizontal arrangement ("vertically mirrored") (CIRAM A10 = PPU A10)
 * ||||||+-- 1: Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory
 * |||||+--- 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
 * ||||+---- 1: Alternative nametable layout
 * ++++----- Lower nybble of mapper number
 *
 * Reference:
 * https://www.nesdev.org/wiki/INES#Flags_6
 */
enum nes_ines_attr_flags_6 {
	NESEMU_INES_FLAGS_6_NAMETABLE = 0x01,
	NESEMU_INES_FLAGS_6_BATTERY = 0x02,
	NESEMU_INES_FLAGS_6_TRAINER = 0x04,
	NESEMU_INES_FLAGS_6_ALT = 0x08,
	NESEMU_INES_FLAGS_6_MAPPER_LNYBBLE = 0xF0,
};

/**
 * Flags 7 from the iNES header bit masks
 *
 * 76543210
 * ||||||||
 * |||||||+- VS Unisystem
 * ||||||+-- PlayChoice-10 (8 KB of Hint Screen data stored after CHR data)
 * ||||++--- If equal to 2, flags 8-15 are in NES 2.0 format
 * ++++----- Upper nybble of mapper number
 *
 * Reference:
 * https://www.nesdev.org/wiki/INES#Flags_7
 */
enum nes_ines_attr_flags_7 {
	NESEMU_INES_FLAGS_7_VS = 0x01,
	NESEMU_INES_FLAGS_7_PC = 0x02,
	NESEMU_INES_FLAGS_7_INES2 = 0x0C,
	NESEMU_INES_FLAGS_7_MAPPER_HNYBBLE = 0xF0,
};

/**
 * iNES cartridge with appropiate mapper callbacks
 *
 * In the original NES hardware, some addresses are mapped directly to the
 * cartridge. In order to provide this functionality, the cartridge structure
 * includes some `callback` function pointers, this callbacks will be called
 * by the `memory` subsystem to read this addresses.
 *
 * Beware that CHRROM/CHRRAM should always return `
 */
typedef struct nes_cartridge_t {

	/** Variant discriminator */
	enum nes_ines_mapper_variant type;

	/** Variant type */
	union nes_ines_mapper_t mapper;

	/* -- Mapper Callbacks -- */

	/**
     * Callback method to load program data into the cartridge struct.
     * Should be called at an offset (no header data nor chrrom data)
     *
     * @note Required field, must not be NULL.
     */
	nes_cartridge_loader_t prg_load_fn;

	/**
     * Callback method to read program data from cartridge.
     *
     * @note Required field, must not be NULL.
     */
	nes_cartridge_read_t prg_read_fn;

	/**
     * Callback method to write program data into cartridge.
     *
     * @note Required field, must not be NULL.
     */
	nes_cartridge_write_t prg_write_fn;

	/**
     * Method to load video data into the cartridge struct
     * Should be called at an offset (no header data nor prgrom data)
     *
     * @note Required field, must not be NULL.
     */
	nes_cartridge_loader_t chr_load_fn;

	/**
     * Callback method to read from cartridge's chrrom/chrram.
     * @note Required field, must not be NULL.
     */
	nes_cartridge_read_t chr_read_fn;

	/**
     * Callback method to map VRAM addresses (Nametable mirroring).
     *
     * @note Required field, must not be NULL.
     */
	nes_cartridge_mapper_t chr_mapper_fn;

	/**
     * Method to write into cartridge's chrram.
     *
     * If this method is not null, then cartridge is assumed to have CHRRAM section
     *
     * @note Optional field, leave as NULL when no CHRRAM is available.
     */
	nes_cartridge_write_t chr_write_fn;

} nes_cartridge_t;

/**
 * Get a generic reference to the mapper internal structure.
 * Use this as first parameter of the mapper callbacks (type punning)
 *
 * @param c (struct nes_cartridge_t *) cartridge reference
 */
#define NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(c) \
	(nesemu_mapper_generic_ref_t) & c->mapper.__self

/**
 * Load iNES cartridge into memory
 *
 * @note This is the constructor/initializer for the `struct nes_cartridge_t`
 * structure.
 * @note Do this before the CPU is initialized.
 */
nesemu_return_t nes_cartridge_read_ines(struct nes_cartridge_t *cartridge,
					uint8_t *data,
					size_t len);

#endif
