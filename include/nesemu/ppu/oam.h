#ifndef __NESEMU_PPU_OAM_H__
#define __NESEMU_PPU_OAM_H__

#include <stdint.h>

/** Amount of sprites that fit into OAM */
#define NESEMU_PPU_OAM_SPRITES 64

/** Amount of sprites that fit into secondary OAM */
#define NESEMU_PPU_SOAM_SPRITES 8

/**
 * Structure of a single entry in the OAM
 */
struct nes_ppu_oam {
	uint8_t y;
	uint8_t tile;
	uint8_t attr;
	uint8_t x;
} __attribute__((packed));

#endif
