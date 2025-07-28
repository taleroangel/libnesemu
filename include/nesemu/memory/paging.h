#ifndef __NESEMU_MEMORY_PAGING_H__
#define __NESEMU_MEMORY_PAGING_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * Base address for zero page
 */
#define NESEMU_ZEROPAGE_BASE_ADDR 0x00

/**
 * Transform a zeropage address into a raw memory address
 */
#define NESEMU_ZEROPAGE_GET_ADDR(addr) (uint16_t)(0x00FF & addr)

/**
 * Check if an address is page crossed
 */
static inline bool nes_mem_is_crosspage(uint16_t addr) {
    return (addr & 0x00FF) == 0xFF;
}

#endif
