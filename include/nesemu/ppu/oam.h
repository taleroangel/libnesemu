#ifndef __NESEMU_PPU_OAM_H__
#define __NESEMU_PPU_OAM_H__

#include <stdint.h>

/** Amount of sprites that fit into OAM */
#define NESEMU_PPU_OAM_SPRITES 64

/**
 * Structure of a single entry in the OAM
 */
struct nes_sprite_data_t {
	uint8_t y;
	uint8_t tile;
	uint8_t attr;
	uint8_t x;
} __attribute__((packed));

/** Object Attrubute Memory (OAM) */
typedef struct nes_sprite_data_t nes_oam_t[NESEMU_PPU_OAM_SPRITES];

#endif
