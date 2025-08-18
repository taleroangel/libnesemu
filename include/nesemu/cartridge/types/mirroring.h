/**
 * Common nametable mirroring functions.
 *
 * Reference:
 * https://www.nesdev.org/wiki/Mirroring#Nametable_Mirroring
 */

#ifndef __NESEMU_CARTRIDGE_MIRRORING_H__
#define __NESEMU_CARTRIDGE_MIRRORING_H__

#include "common.h"

/**
 * Horizontal nametable mirroring.
 *
 * $2000 -> $2000
 * $2400 -> $2000
 * $2800 -> $2400
 * $2C00 -> $2400.
 *
 * @note CHRROM mapped to cartridge
 */
nesemu_return_t nes_cartridge_mapper_horizontal(nesemu_mapper_generic_ref_t self,
						uint16_t addr,
						uint16_t *mapped);
/*
 * Vertical nametable mirroring
 *
 * @note CHRROM mapped to cartridge
 */
nesemu_return_t nes_cartridge_mapper_vertical(nesemu_mapper_generic_ref_t self,
					      uint16_t addr,
					      uint16_t *mapped);

#endif
