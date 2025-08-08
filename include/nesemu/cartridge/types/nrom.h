#ifndef __NESEMU_CARTRIDGE_TYPES_NROM_H__
#define __NESEMU_CARTRIDGE_TYPES_NROM_H__

#include "common.h"

#include "nesemu/util/error.h"

#include <stdint.h>
#include <string.h>

/**
 * NROM cartridge memory layout
 */
struct nes_ines_nrom_cartridge_t {
	uint8_t bank[NESEMU_CARTRIDGE_BANK_SIZE]; /**< Only memory bank in cartridge */
};

nesemu_error_t nes_ines_nrom_cpu_loader(nesemu_mapper_generic_ref_t self,
					uint8_t *cdata,
					size_t len)
{
	// Create a reference to `nes_ines_nrom_cartridge_t` called `this`
	NESEMU_CARTRIDGE_DEFINE_GENERIC_TYPE(
		this, struct nes_ines_nrom_cartridge_t, self);

	// Copy memory from cartridge into `this`
	(void)memcpy(this->bank, cdata, len);
	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_ines_nrom_cpu_reader(nesemu_mapper_generic_ref_t self,
					uint16_t addr,
					uint8_t *content)
{
}

nesemu_error_t nes_ines_nrom_cpu_writer(nesemu_mapper_generic_ref_t self,

					uint16_t addr,
					uint8_t content)
{
}

#endif
