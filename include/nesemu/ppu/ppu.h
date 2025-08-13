/**
 * NES PPU related code
 *
 * Reference
 * https://www.nesdev.org/wiki/PPU
 */

#ifndef __NESEMU_PPU_H__
#define __NESEMU_PPU_H__

/**
 * NES PPU registers
 *
 * Reference:
 * https://www.nesdev.org/wiki/PPU_registers
 */
enum nes_ppu_registers_t {
	NESEMU_PPU_REG_PPUCTRL = 0x2000,
	NESEMU_PPU_REG_PPUMASK = 0x2001,
	NESEMU_PPU_REG_PPUSTATUS = 0x2002,
	NESEMU_PPU_REG_OAMADDR = 0x2003,
	NESEMU_PPU_REG_OAMDATA = 0x2004,
	NESEMU_PPU_REG_PPUSCROLL = 0x2005,
	NESEMU_PPU_REG_PPUADDR = 0x2006,
	NESEMU_PPU_REG_PPUDATA = 0x2007,
	NESEMU_PPU_REG_OAMDMA = 0x4014,
};

#endif
