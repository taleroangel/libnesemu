/**
 * References:
 * https://www.nesdev.org/wiki/Stack
 */

#ifndef __NESEMU_MEMORY_STACK_H__
#define __NESEMU_MEMORY_STACK_H__

#include "nesemu/memory/main.h"
#include "nesemu/util/error.h"

#include <stdint.h>

/**
 * Start address for the stack in memory
 */
#define NESEMU_STACK_BASE_ADDR 0x0100

/**
 * Transform a stack address into a raw memory address by adding the base
 * address to the given value
 */
#define NESEMU_STACK_GET_ADDR(addr) \
	(uint8_t)(NESEMU_STACK_BASE_ADDR + (uint16_t)addr)

/**
 * Max stack size in memory
 */
#define NESEMU_STACK_SIZE 0xFF

/* Check memory mappings */
#if NESEMU_MEMORY_RAM_SIZE < (NESEMU_STACK_BASE_ADDR + NESEMU_STACK_SIZE)
#error "Memory is not big enough to host stack, check stack and memory mappings"
#endif

/**
 * Push a word to the stack
 *
 * @param mem Memory array
 * @param sp Reference to the stack pointer
 * @param value Value to be pushed
 */
nesemu_return_t nes_stack_push_u8(struct nes_mem_main *mem,
				 uint8_t *sp,
				 uint8_t value);

/**
 * Pull a word from the stack
 * 
 * @param mem Memory array
 * @param sp Reference to the stack pointer
 * @param value Reference where the value will be stored 
 */
nesemu_return_t nes_stack_pop_u8(struct nes_mem_main *mem,
				uint8_t *sp,
				uint8_t *result);

/**
 * Push a dword to the stack
 *
 * @param mem Memory array
 * @param sp Reference to the stack pointer
 * @param value Value to be pushed
 */
nesemu_return_t nes_stack_push_u16(struct nes_mem_main *mem,
				  uint8_t *sp,
				  uint16_t value);

/**
 * Pull a dword from the stack
 * 
 * @param mem Memory array
 * @param sp Reference to the stack pointer
 * @param value Reference where the value will be stored 
 */
nesemu_return_t nes_stack_pop_u16(struct nes_mem_main *mem,
				 uint8_t *sp,
				 uint16_t *result);

#endif
