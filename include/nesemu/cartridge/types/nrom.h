#ifndef __NESEMU_CARTRIDGE_TYPES_NROM_H__
#define __NESEMU_CARTRIDGE_TYPES_NROM_H__

#include "common.h"

#include "nesemu/util/error.h"

#include <stdint.h>
#include <string.h>

/** Size of the PRGRAM in cartridge */
#define NESEMU_CARTRIDGE_NROM_PRGROM_SIZE (NESEMU_CARTRIDGE_PRGROM_BANK_SIZE)

/** Size of the CHRROM in cartridge */
#define NESEMU_CARTRIDGE_NROM_CHRROM_SIZE (NESEMU_CARTRIDGE_CHRROM_BANK_SIZE)

/**
 * NROM cartridge memory layout
 */
struct nes_ines_nrom_cartridge {
	/**
     * Single memory bank for PRGROM in the cartridge.
     */
	uint8_t prgrom[NESEMU_CARTRIDGE_NROM_PRGROM_SIZE];

	/**
     * Single memory bank for CHRROM in the cartridge.
     */
	uint8_t chrrom[NESEMU_CARTRIDGE_NROM_CHRROM_SIZE];
};

/* -- Callbacks -- */

nesemu_return_t nes_ines_nrom_prg_loader(struct nes_ines_nrom_cartridge *self,
					 uint8_t *cdata,
					 size_t len);

nesemu_return_t nes_ines_nrom_prg_reader(struct nes_ines_nrom_cartridge *self,
					 uint16_t addr,
					 uint8_t *content);

/* No PRG writer as there is no external vram nor PRGRAM in this mapping */
#define nes_ines_nrom_prg_writer NULL;

nesemu_return_t nes_ines_nrom_chr_loader(struct nes_ines_nrom_cartridge *self,
					 uint8_t *cdata,
					 size_t len);

nesemu_return_t nes_ines_nrom_chr_reader(struct nes_ines_nrom_cartridge *self,
					 uint16_t addr,
					 uint8_t *content);

/* No CHR writer as there is no external vram nor CHRRAM in this mapping */
#define nes_ines_nrom_chr_writer NULL;

/* 
 * Mirroring can be either horizontal or vertical.
 * This should be set manually by the initialization code based on
 * cartridge parameters
 */
#define nes_ines_nrom_chr_mapper NULL;

#endif
