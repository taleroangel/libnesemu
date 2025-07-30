#ifndef __NESEMU_MEMORY_ROM_H__
#define __NESEMU_MEMORY_ROM_H__

#include "nesemu/memory/memory.h"
#include "nesemu/util/error.h"

#include <stddef.h>
#include <stdint.h>

/**
 * Address at which the ROM begins in memory
 * This initial section is mapped to cardrige RAM
 */
#define NESEMU_MEMORY_ROM_BEGIN (uint16_t)0x4020

/**
 * Address at which PRGROM (cardrige ROM) begins in memory
 */
#define NESEMU_MEMORY_PRGROM_BEGIN (uint16_t)0x8000

/**
 * Max size for PRGROM
 */
#define NESEMU_MEMORY_PRGROM_SIZE \
    (size_t)(NESEMU_MEMORY_SIZE - NESEMU_MEMORY_PRGROM_BEGIN) 

/**
 * Write PRGROM into memory.
 * This is the data containing the program
 */
nesemu_error_t nes_memory_wPRGROM(nes_memory_t mem, uint8_t *data, size_t len);

#endif
