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
	/**
     * Single memory bank for PRGROM in the cartridge.
     * Use `__NESEMU_CARTRIDGE_NROM_GET_ADDR` to map addresses to this array
     */
	uint8_t prgrom[NESEMU_CARTRIDGE_BANK_SIZE];
};

nesemu_error_t nes_ines_nrom_cpu_loader(nesemu_mapper_generic_ref_t self,
					uint8_t *cdata,
					size_t len);

nesemu_error_t nes_ines_nrom_cpu_reader(nesemu_mapper_generic_ref_t self,
					uint16_t addr,
					uint8_t *content);

nesemu_error_t nes_ines_nrom_cpu_writer(nesemu_mapper_generic_ref_t self,
					uint16_t addr,
					uint8_t content);

#endif
