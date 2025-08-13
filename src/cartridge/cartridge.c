/* -- Include Cartridge Types -- */
/* Because of the helper macro `_CARTRIDGE_CALLBACKS_FOR_TYPE_BOILERPLATE`.
 * Some linters may complain about unused headers, make sure to disable this
 * kind of warning for this file */

#include "nesemu/cartridge/cartridge.h"
#include "nesemu/cartridge/types/nrom.h" /* IWYU pragma: keep */

/* -- Other includes -- */

#include "nesemu/util/compat.h"
#include "nesemu/util/error.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * iNES header initial character set
 */
static const char iNES_header[] = { 'N', 'E', 'S', 0x1A };

/**
 * Helper macro to reduce boilerplate when defining cartridge callbacks
 * for a given cartridge type (i.e 'nrom')
 *
 * This will only work if the cartridge callbacks are named with the
 * following format:
 *
 *      nes_ines_<type>_prg_loader,
 *      nes_ines_<type>_prg_reader,
 *      nes_ines_<type>_prg_writer,
 *      nes_ines_<type>_chr_loader,
 *      nes_ines_<type>_chr_reader,
 *      nes_ines_<type>_chr_writer
 */
#define _CARTRIDGE_CALLBACKS_FOR_TYPE_BOILERPLATE(cartridge, type) \
	cartridge->prg_load_fn = nes_ines_##type##_prg_loader;     \
	cartridge->prg_read_fn = nes_ines_##type##_prg_reader;     \
	cartridge->prg_write_fn = nes_ines_##type##_prg_writer;    \
	cartridge->chr_load_fn = nes_ines_##type##_chr_loader;     \
	cartridge->chr_write_fn = nes_ines_##type##_chr_writer;

/* TODO load CHRROM and validate other header options */
nesemu_error_t nes_read_ines(struct nes_cartridge_t *cartridge,
			     uint8_t *data,
			     size_t len)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (len < NESEMU_CARTRIDGE_INES_HEADER_SIZE) {
		return NESEMU_RETURN_CARTRIDGE_BAD_INES_FORMAT;
	}
#endif

	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Validate NES header
	if (strncmp((const char *)data, iNES_header, sizeof(iNES_header)) !=
	    0) {
		return NESEMU_RETURN_CARTRIDGE_BAD_INES_FORMAT;
	}

	// Get PRGROM size
	size_t prgrom_len =
		data[NESEMU_CARTRIDGE_INES_HEADER_PRGROM_SIZE_INDEX] *
		NESEMU_CARTRIDGE_INES_HEADER_PRGROM_CHUNK_SIZE;

	// Get CHRROM size
	size_t chrrom_len =
		data[NESEMU_CARTRIDGE_INES_HEADER_CHRROM_SIZE_INDEX] *
		NESEMU_CARTRIDGE_INES_HEADER_CHRROM_CHUNK_SIZE;

	// Get pointer to cartridge data
	uint8_t *cdata = data + NESEMU_CARTRIDGE_INES_HEADER_SIZE;
	len -= NESEMU_CARTRIDGE_INES_HEADER_SIZE;

#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	// Check if cartridge is not empty
	if (len == 0) {
		return NESEMU_RETURN_CARTRIDGE_EMPTY;
	}
#endif

	/* ! -- Mapper Setup -- */
	cartridge->type = data[NESEMU_CARTRIDGE_INES_HEADER_MAPPER_TYPE_INDEX];

	switch (cartridge->type) {
	// NROM mapper
	case NESEMU_INES_MAPPER_NROM:
		_CARTRIDGE_CALLBACKS_FOR_TYPE_BOILERPLATE(cartridge, nrom);
		break;

	// Unsupported mappers
	case NESEMU_INES_MAPPER_UNSUPPORTED:
		_NESEMU_FALLTHROUGH;
	default:
		return NESEMU_RETURN_CARTRIDGE_UNSUPPORTED_MAPPER;
	}

	// Delegate to cartridge loader
	if ((err = cartridge->prg_load_fn(&cartridge->mapper, cdata,
					  prgrom_len)) !=
	    NESEMU_RETURN_SUCCESS) {
		return err;
	}

	return err;
}
