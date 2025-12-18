#ifndef __NESEMU_INCLUDE_H__
#define __NESEMU_INCLUDE_H__

/* All includes */
#include <nesemu/util/error.h>
#include <nesemu/cartridge/cartridge.h>
#include <nesemu/memory/main.h>
#include <nesemu/memory/video.h>
#include <nesemu/cpu/cpu.h>
#include <nesemu/ppu/ppu.h>

/** NES screen height */
#define NESEMU_HEIGHT NESEMU_PPU_SCREEN_HEIGHT

/** NES screen width */
#define NESEMU_WIDTH NESEMU_PPU_SCREEN_WIDTH
 
#endif
