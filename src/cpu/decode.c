/**
 * This file contains definitions for functions in 'cpu.h'
 * Contains code required to decode CPU instructions
 */

#include "nesemu/cpu/cpu.h"
#include "nesemu/cpu/instructions.h"
#include "nesemu/cpu/status.h"

#include "nesemu/memory/memory.h"
#include "nesemu/memory/paging.h"
#include "nesemu/memory/stack.h"

#include "nesemu/util/error.h"
#include "nesemu/util/bits.h"
#include "nesemu/util/compat.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

/* Private Functions */

/**
 * Read appropiate memory address given the addressing mode
 *
 * @note Function to reduce boilerplate
 * @note Will read from actual prgmem, so $pc will change
 * @note NESEMU_ADDRESSING_IMMEDIATE not supported! will return error
 *
 * @param addr Reference to where the memory address to be used is stored.
 */
static nesemu_error_t cpu_read_addr(struct nes_cpu_t *self,
				    enum nes_cpu_addressing_mode_t addressing,
				    nes_memory_t mem,
				    uint16_t *addr)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing addresses
	uint8_t lsb = 0, msb = 0;

	// Placeholder for a full address and pointer for indirect mode
	uint16_t ptr = 0;

	// Address
	*addr = 0;

	switch (addressing) {
	case NESEMU_ADDRESSING_ACCUMULATOR:
		_NESEMU_FALLTHROUGH;
	case NESEMU_ADDRESSING_IMMEDIATE:
		return NESEMU_RETURN_CPU_BAD_ADDRESSING;

	case NESEMU_ADDRESSING_ZERO_PAGE:
		// Zero Page
		*addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		break;

	case NESEMU_ADDRESSING_ZERO_PAGE_X:
		// Zero Page and add X register to addr
		*addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
						 self->x);
		break;

	case NESEMU_ADDRESSING_ZERO_PAGE_Y:
		// Zero Page and add Y register to addr
		*addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
						 self->y);
		break;

	case NESEMU_ADDRESSING_ABSOLUTE:
		// Absolute
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		*addr = NESEMU_UTIL_U16(msb, lsb);
		break;

	case NESEMU_ADDRESSING_ABSOLUTE_X:
		// Absolute, X
		// Load contents at memory address and add X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		*addr = NESEMU_UTIL_U16(msb, lsb) + self->x;
		break;

	case NESEMU_ADDRESSING_ABSOLUTE_Y:
		// Absolute, Y
		// Load contents at memory address and add Y
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		*addr = NESEMU_UTIL_U16(msb, lsb) + self->y;
		break;

		/* Indirect addressing should be handled manually within JMP instruction
     * due to a CPU bug that need to be simulated.
     *
     * Adding that code here would cause a small overhead for other operations
     * and thus moved there instead.
     */
	case NESEMU_ADDRESSING_INDIRECT:
		return NESEMU_RETURN_MEMORY_INVALILD_ADDR;

	case NESEMU_ADDRESSING_INDIRECT_X:
		// Indirect X (Pre-Indexed)
		// Load the pointer address (always zero page) and add X
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
					       self->x);
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, addr);
		break;

	case NESEMU_ADDRESSING_INDIRECT_Y:
		// Indirect Y (Post-Indexed)
		// Load the pointer address (always zero page)
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, addr);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		// Add Y to the actual address
		addr += self->y;
		break;
	}

	// Return the error
	return err;
}

/**
 * Read an u8 value from memory using appropiate addressing mode
 *
 * @note Function to reduce boilerplate
 * @note Will call 'get_addr' so $pc will change
 * @note Will add 1 to 'cycles' in some addressing modes with crosspage
 *
 * @param cycles Reference to the CPU cycles that the operation will take,
 * this value will change depending on the addr mode
 * @param memory Reference to where the value will be stored
 */
static nesemu_error_t cpu_read_mem(struct nes_cpu_t *self,
				   enum nes_cpu_addressing_mode_t addressing,
				   nes_memory_t mem,
				   int *cycles,
				   uint8_t *memory)
{
	// Return status
	nesemu_error_t err = 0;

	// Store address
	uint16_t addr = 0;

	switch (addressing) {
	case NESEMU_ADDRESSING_ACCUMULATOR:
		return NESEMU_RETURN_CPU_BAD_ADDRESSING;

	case NESEMU_ADDRESSING_IMMEDIATE:
		*memory = nes_cpu_fetch(self, mem);
		break;

	// Additional cycle if page crossed
	case NESEMU_ADDRESSING_ABSOLUTE_X:
	case NESEMU_ADDRESSING_ABSOLUTE_Y:
	case NESEMU_ADDRESSING_INDIRECT_Y:
		if (nes_mem_is_crosspage(addr)) {
			*cycles += 1;
		}
		_NESEMU_FALLTHROUGH;
	default:
		// Get the address
		err = cpu_read_addr(self, addressing, mem, &addr);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}

		// Read value
		err = nes_mem_r8(mem, addr, memory);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;
	}

	return err;
}

static nesemu_error_t _LDA(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Get value and store it in A
	nesemu_error_t err =
		cpu_read_mem(self, addressing, mem, cycles, &self->a);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->a));

	return err;
}

static nesemu_error_t _LDX(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Get value and store it in X
	nesemu_error_t err =
		cpu_read_mem(self, addressing, mem, cycles, &self->x);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->x));

	return err;
}

static nesemu_error_t _LDY(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Get value and store it in Y
	nesemu_error_t err =
		cpu_read_mem(self, addressing, mem, cycles, &self->y);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->y));

	return err;
}

static nesemu_error_t _STA(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Get the address
	err = cpu_read_addr(self, addressing, mem, &addr);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Write value
	err = nes_mem_w8(mem, addr, self->a);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	return err;
}

static nesemu_error_t _STX(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Get the address
	err = cpu_read_addr(self, addressing, mem, &addr);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Write value
	err = nes_mem_w8(mem, addr, self->x);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	return err;
}

static nesemu_error_t _STY(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Get the address
	err = cpu_read_addr(self, addressing, mem, &addr);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Write value
	err = nes_mem_w8(mem, addr, self->y);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	return err;
}

static nesemu_error_t _TXX(struct nes_cpu_t *self, uint8_t opcode)
{
	switch (opcode) {
	case TAX:
		self->x = self->a;
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->x));
		break;

	case TXA:
		self->a = self->x;
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		break;

	case TAY:
		self->y = self->a;
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->y));
		break;

	case TYA:
		self->a = self->y;
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		break;

	case TSX:
		self->x = self->sp;
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->x));
		break;

	case TXS:
		self->sp = self->x;
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	return NESEMU_RETURN_SUCCESS;
}

static nesemu_error_t _ADC(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Where to store the value to be added
	uint8_t memory = 0;

	// Read memory given addressing mode
	nesemu_error_t err =
		cpu_read_mem(self, addressing, mem, cycles, &memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Get result of operation
	uint16_t result =
		// a + m + c
		self->a + memory + (self->status & NESEMU_CPU_FLAGS_C);

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->a));

	// Clear/Set carry (C)
	(result > UINT8_MAX) ?
		// Overflow, set carry
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C) :
		// No overflow, clear carry
		nes_cpu_status_mask_unset(self, NESEMU_CPU_FLAGS_C);

	// Clear/Set overflow (V)
	(((uint8_t)result ^ self->a) & ((uint8_t)result ^ memory) &
	 NESEMU_CPU_FLAGS_N) ?
		// Set overflow
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_V) :
		// Unset overflow
		nes_cpu_status_mask_unset(self, NESEMU_CPU_FLAGS_V);

	// Store result
	self->a = (uint8_t)(result & 0x00FF);

	return err;
}

static nesemu_error_t _SBC(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Where to store the value to be added
	uint8_t memory = 0;

	// Read memory given addressing mode
	nesemu_error_t err =
		cpu_read_mem(self, addressing, mem, cycles, &memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Get result of operation (signed)
	int16_t result =
		// a - m - ~c
		self->a - memory - ~(self->status & NESEMU_CPU_FLAGS_C);

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->a));

	// Clear/Set carry (C)
	(result < 0) ?
		// Underflow, set carry
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C) :
		// No underflow, clear carry
		nes_cpu_status_mask_unset(self, NESEMU_CPU_FLAGS_C);

	// Clear/Set overflow (V)
	(((uint8_t)result ^ self->a) & ((uint8_t)result ^ ~memory) &
	 NESEMU_CPU_FLAGS_N) ?
		// Set overflow
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_V) :
		// Unset overflow
		nes_cpu_status_mask_unset(self, NESEMU_CPU_FLAGS_V);

	// Store result
	self->a = (uint8_t)((uint16_t)result & 0x00FF);

	return err;
}

static nesemu_error_t _INC(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Placeholder for memory
	uint8_t memory = 0;

	// Get the address
	err = cpu_read_addr(self, addressing, mem, &addr);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Read value
	err = nes_mem_r8(mem, addr, &memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Update value
	memory += 1;

	// Store value
	err = nes_mem_w8(mem, addr, memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(memory));

	return err;
}

static nesemu_error_t _DEC(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Placeholder for memory
	uint8_t memory = 0;

	// Get the address
	err = cpu_read_addr(self, addressing, mem, &addr);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Read value
	err = nes_mem_r8(mem, addr, &memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Update value
	memory -= 1;

	// Store value
	err = nes_mem_w8(mem, addr, memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(memory));

	return err;
}

static inline void _INX(struct nes_cpu_t *self)
{
	// Increment the register
	self->x += 1;

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->x));
}

static inline void _DEX(struct nes_cpu_t *self)
{
	// Decrement the register
	self->x -= 1;

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->x));
}

static inline void _INY(struct nes_cpu_t *self)
{
	// Increment the register
	self->y += 1;

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->y));
}

static inline void _DEY(struct nes_cpu_t *self)
{
	// Decrement the register
	self->y -= 1;

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->y));
}

static nesemu_error_t _ASL(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Placeholder for memory
	uint8_t memory = 0;

	switch (addressing) {
		// Use the accumulator instead
	case NESEMU_ADDRESSING_ACCUMULATOR:
		memory = self->a;
		break;

	default:
		// Get the address
		err = cpu_read_addr(self, addressing, mem, &addr);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		// Read value
		err = nes_mem_r8(mem, addr, &memory);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;
	}

	// Update value
	uint16_t result = (uint16_t)memory << 1;

	// Store u8 result without first bit
	memory = (uint8_t)result & 0xFE;

	switch (addressing) {
		// Use the accumulator instead
	case NESEMU_ADDRESSING_ACCUMULATOR:
		self->a = memory;
		break;

	default:
		// Store value
		err = nes_mem_w8(mem, addr, memory);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;
	}

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(memory));

	// Update carry, check if the result had an overflow
	(result & 0x0100) ? nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C) :
			    nes_cpu_status_mask_unset(self, NESEMU_CPU_FLAGS_C);

	return err;
}

static nesemu_error_t _LSR(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Placeholder for memory
	uint8_t memory = 0;

	switch (addressing) {
		// Use the accumulator instead
	case NESEMU_ADDRESSING_ACCUMULATOR:
		memory = self->a;
		break;

	default:
		// Get the address
		err = cpu_read_addr(self, addressing, mem, &addr);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		// Read value
		err = nes_mem_r8(mem, addr, &memory);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;
	}

	// Shift value (using MSB of u16)
	uint16_t result = ((uint16_t)memory << 8) >> 1;

	// Store u8 part of the result's MSB without 7th bit
	memory = (uint8_t)(result >> 8) & 0x7F;

	switch (addressing) {
		// Use the accumulator instead
	case NESEMU_ADDRESSING_ACCUMULATOR:
		self->a = memory;
		break;

	default:
		// Store value
		err = nes_mem_w8(mem, addr, memory);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;
	}

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(memory));

	// Update carry, check if the result had an overflow
	(result & 0x0080) ? nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C) :
			    nes_cpu_status_mask_unset(self, NESEMU_CPU_FLAGS_C);

	return err;
}

static nesemu_error_t _ROL(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Placeholder for memory
	uint8_t memory = 0;

	switch (addressing) {
		// Use the accumulator instead
	case NESEMU_ADDRESSING_ACCUMULATOR:
		memory = self->a;
		break;

	default:
		// Get the address
		err = cpu_read_addr(self, addressing, mem, &addr);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		// Read value
		err = nes_mem_r8(mem, addr, &memory);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;
	}

	// C <- [76543210] <- C
	memory = (memory << 1) | ((memory | 0x80) >> 7);

	switch (addressing) {
		// Use the accumulator instead
	case NESEMU_ADDRESSING_ACCUMULATOR:
		self->a = memory;
		break;

	default:
		// Store value
		err = nes_mem_w8(mem, addr, memory);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;
	}

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(memory));

	// Update carry, copy value of first bit
	(memory & 0x01) ? nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C) :
			  nes_cpu_status_mask_unset(self, NESEMU_CPU_FLAGS_C);

	return err;
}

static nesemu_error_t _ROR(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Placeholder for memory
	uint8_t memory = 0;

	switch (addressing) {
		// Use the accumulator instead
	case NESEMU_ADDRESSING_ACCUMULATOR:
		memory = self->a;
		break;

	default:
		// Get the address
		err = cpu_read_addr(self, addressing, mem, &addr);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		// Read value
		err = nes_mem_r8(mem, addr, &memory);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;
	}

	// C -> [76543210] -> C
	memory = (memory >> 1) | ((memory | 0x01) << 7);

	switch (addressing) {
		// Use the accumulator instead
	case NESEMU_ADDRESSING_ACCUMULATOR:
		self->a = memory;
		break;

	default:
		// Store value
		err = nes_mem_w8(mem, addr, memory);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;
	}

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(memory));

	// Update carry, copy value of first bit
	(memory & 0x80) ? nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C) :
			  nes_cpu_status_mask_unset(self, NESEMU_CPU_FLAGS_C);

	return err;
}

static nesemu_error_t _AND(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Place holder
	uint8_t memory = 0;

	// Read memory given addressing mode
	nesemu_error_t err =
		cpu_read_mem(self, addressing, mem, cycles, &memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Operation
	self->a &= memory;

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->a));

	return err;
}

static nesemu_error_t _ORA(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Place holder
	uint8_t memory = 0;

	// Read memory given addressing mode
	nesemu_error_t err =
		cpu_read_mem(self, addressing, mem, cycles, &memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Operation
	self->a |= memory;

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->a));

	return err;
}

static nesemu_error_t _EOR(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Place holder
	uint8_t memory = 0;

	// Read memory given addressing mode
	nesemu_error_t err =
		cpu_read_mem(self, addressing, mem, cycles, &memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Operation
	self->a ^= memory;

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->a));

	return err;
}

static nesemu_error_t _BIT(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Place holder
	uint8_t memory = 0;

	// Get the address
	err = cpu_read_addr(self, addressing, mem, &addr);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Read value
	err = nes_mem_r8(mem, addr, &memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Operation
	uint8_t result = self->a & memory;

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_Z(result));
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_N(memory));
	nes_cpu_status_mask_set(self, memory & NESEMU_CPU_FLAGS_V);

	return err;
}

static nesemu_error_t _CMP(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Place holder
	uint8_t memory = 0;

	// Read memory given addressing mode
	nesemu_error_t err =
		cpu_read_mem(self, addressing, mem, cycles, &memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Operation
	uint8_t result = self->a - memory;

	// Update status
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(result));

	// Carry
	(self->a >= memory) ?
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C) :
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C);

	return err;
}

static nesemu_error_t _CPX(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Place holder
	uint8_t memory = 0;

	// Read memory given addressing mode
	nesemu_error_t err =
		cpu_read_mem(self, addressing, mem, cycles, &memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Operation
	uint8_t result = self->x - memory;

	// Update status
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(result));

	// Carry
	(self->x >= memory) ?
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C) :
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C);

	return err;
}

static nesemu_error_t _CPY(struct nes_cpu_t *self,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Place holder
	uint8_t memory = 0;

	// Read memory given addressing mode
	nesemu_error_t err =
		cpu_read_mem(self, addressing, mem, cycles, &memory);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Operation
	uint8_t result = self->y - memory;

	// Update status
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(result));

	// Carry
	(self->y >= memory) ?
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C) :
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C);

	return err;
}

static inline nesemu_error_t _PHA(struct nes_cpu_t *self, nes_memory_t mem)
{
	return nes_stack_push_u8(mem, &self->sp, self->a);
}

static inline nesemu_error_t _PLA(struct nes_cpu_t *self, nes_memory_t mem)
{
	nesemu_error_t err = nes_stack_pop_u8(mem, &self->sp, &self->a);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->a));
	return err;
}

static inline nesemu_error_t _PHP(struct nes_cpu_t *self, nes_memory_t mem)
{
	uint8_t status =
		NESEMU_CPU_STATUS_SET_MASK(self->status, NESEMU_CPU_FLAGS_B);
	return nes_stack_push_u8(mem, &self->sp, status);
}

static inline nesemu_error_t _PLP(struct nes_cpu_t *self, nes_memory_t mem)
{
	// Read status
	uint8_t status = 0;
	nesemu_error_t err = nes_stack_pop_u8(mem, &self->sp, &status);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Store status (ignore B flag)
	self->status = (status & ~NESEMU_CPU_FLAGS_B) |
		       (self->status & NESEMU_CPU_FLAGS_B);

	return err;
}

static nesemu_error_t _CXX_SXX(struct nes_cpu_t *self, uint8_t opcode)
{
	switch (opcode) {
	case CLC:
		nes_cpu_status_mask_unset(self, NESEMU_CPU_FLAGS_C);
		break;

	case SEC:
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C);
		break;

	case CLI:
		nes_cpu_status_mask_unset(self, NESEMU_CPU_FLAGS_I);
		break;

	case SEI:
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_I);
		break;

	case CLD:
		nes_cpu_status_mask_unset(self, NESEMU_CPU_FLAGS_D);
		break;

	case SED:
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_D);
		break;

	case CLV:
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_V);
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	return NESEMU_RETURN_SUCCESS;
}

static void _BXX(struct nes_cpu_t *self,
		 nes_memory_t mem,
		 int *cycles,
		 bool condition)
{
	// Amount to jump
	int8_t jrelative = (int8_t)nes_cpu_fetch(self, mem);

	// Check condition
	if (!condition) {
		// No branching
		return;
	}

	// Branching will occurr
	*cycles += 1;

	// New program counter
	uint16_t pc = self->pc + jrelative;

	// Page boundary crossed
	if ((pc & 0xFF00) != (self->pc & 0xFF00)) {
		*cycles += 1;
	}

	// Set the program counter
	self->pc = pc;
}

static inline void _BCC(struct nes_cpu_t *self, nes_memory_t mem, int *cycles)
{
	// Carry Clear
	_BXX(self, mem, cycles, (self->status & NESEMU_CPU_FLAGS_C) == 0);
}

static inline void _BCS(struct nes_cpu_t *self, nes_memory_t mem, int *cycles)
{
	// Carry Set
	_BXX(self, mem, cycles, (self->status & NESEMU_CPU_FLAGS_C) != 0);
}

static inline void _BEQ(struct nes_cpu_t *self, nes_memory_t mem, int *cycles)
{
	// Zero Set
	_BXX(self, mem, cycles, (self->status & NESEMU_CPU_FLAGS_Z) != 0);
}

static inline void _BNE(struct nes_cpu_t *self, nes_memory_t mem, int *cycles)
{
	// Zero Set
	_BXX(self, mem, cycles, (self->status & NESEMU_CPU_FLAGS_Z) == 0);
}

static inline void _BPL(struct nes_cpu_t *self, nes_memory_t mem, int *cycles)
{
	// Negative is clear
	_BXX(self, mem, cycles, (self->status & NESEMU_CPU_FLAGS_N) == 0);
}

static inline void _BMI(struct nes_cpu_t *self, nes_memory_t mem, int *cycles)
{
	// Negative is set
	_BXX(self, mem, cycles, (self->status & NESEMU_CPU_FLAGS_N) != 0);
}

static inline void _BVC(struct nes_cpu_t *self, nes_memory_t mem, int *cycles)
{
	// Overflow is clear
	_BXX(self, mem, cycles, (self->status & NESEMU_CPU_FLAGS_V) == 0);
}

static inline void _BVS(struct nes_cpu_t *self, nes_memory_t mem, int *cycles)
{
	// Overflow is set
	_BXX(self, mem, cycles, (self->status & NESEMU_CPU_FLAGS_V) != 0);
}

static nesemu_error_t _JMP(struct nes_cpu_t *self,
			   nes_memory_t mem,
			   enum nes_cpu_addressing_mode_t addressing)
{
	// Placeholders
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;
	uint16_t memory = 0, addr = 0, ptr = 0;
	uint8_t lsb = 0, msb = 0;

	// Get memory value
	switch (addressing) {
	case NESEMU_ADDRESSING_ABSOLUTE:
		// Build the value
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		memory = NESEMU_UTIL_U16(msb, lsb);
		break;

	case NESEMU_ADDRESSING_INDIRECT:
		// Build pointer
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		ptr = NESEMU_UTIL_U16(msb, lsb);

		/* BUG Simulation!
         * Reference: https://www.nesdev.org/wiki/Instruction_reference#JMP
         *  
         * The indirect addressing mode uses the operand as a pointer,
         * getting the new 2-byte program counter value from the specified address.
         * Unfortunately, because of a CPU bug, if this 2-byte variable has an
         * address ending in $FF and thus crosses a page, then the CPU fails to
         * increment the page when reading the second byte and thus
         * reads the wrong address.
         * For example, JMP ($03FF) reads $03FF and $0300 instead of $0400.
         */
		if ((ptr & 0x00FF) == 0x00FF) {
			// Read LSB from (ptr)
			err = nes_mem_r8(mem, ptr, &lsb);
			// Read MSB from beginning of the page
			err = nes_mem_r8(mem, (ptr & 0xFF00), &msb);
			// Build the value
			addr = NESEMU_UTIL_U16(msb, lsb);
		} else {
			// Get addr at pointer
			err = nes_mem_r16(mem, ptr, &addr);
		}
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}

		// Read value at addr
		nes_mem_r16(mem, addr, &memory);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;

	default:
		return NESEMU_RETURN_CPU_BAD_ADDRESSING;
	}

	// Set $pc
	self->pc = memory;

	return err;
}

static nesemu_error_t _JSR(struct nes_cpu_t *self, nes_memory_t mem)
{
	// Placeholders
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;
	uint16_t jaddr = 0;
	uint8_t lsb = 0, msb = 0;

	// Build absolute address
	lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
	jaddr = NESEMU_UTIL_U16(msb, lsb);

	// Push $pc to stack
	err = nes_stack_push_u16(mem, &self->sp, self->pc);

	// Set $pc
	self->pc = jaddr;

	return err;
}

static nesemu_error_t _RTS(struct nes_cpu_t *self, nes_memory_t mem)
{
	// Pull from stack
	nesemu_error_t err = nes_stack_pop_u16(mem, &self->sp, &self->pc);

	// Increment the program counter
	self->pc++;

	return err;
}

static nesemu_error_t _BRK(struct nes_cpu_t *self, nes_memory_t mem)
{
	// Store break reason
	self->brkr = nes_cpu_fetch(self, mem);

	// Push program counter
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;
	err = nes_stack_push_u16(mem, &self->sp, self->pc);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Push flags NV11DIZC
	err = nes_stack_push_u16(
		mem, &self->sp,
		NESEMU_CPU_STATUS_SET_MASK(self->status, NESEMU_CPU_FLAGS_B));
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Set Interrupt
	nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_I);

	// Set $pc
	self->pc = NESEMU_CPU_IRQ_ADDR;

	// Build next $pc
	uint8_t lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
	self->pc = NESEMU_UTIL_U16(msb, lsb);

	return err;
}

static nesemu_error_t _RTI(struct nes_cpu_t *self, nes_memory_t mem)
{
	// Placeholders
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Read flags from stack
	uint8_t status = 0;
	err = nes_stack_pop_u8(mem, &self->sp, &status);
	if (err != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Store status (ignore B flag)
	self->status = (status & ~NESEMU_CPU_FLAGS_B) |
		       (self->status & NESEMU_CPU_FLAGS_B);

	// Pull program counter
	err = nes_stack_pop_u16(mem, &self->sp, &self->pc);
	return err;
}

/* Public Functions */

nesemu_error_t nes_cpu_next(struct nes_cpu_t *self, nes_memory_t mem, int *c)
{
	/* Check input arguments */
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (c == NULL) {
	}
#endif
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Get instruction opcode
	uint8_t opc = nes_cpu_fetch(self, mem);

	// Set cycles using the instruction
	*c = nes_cpu_op_cycles[opc];

	// Decode the instruction
	switch (opc) {
	// LDA
	case LDA_IM:
		err = _LDA(self, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case LDA_ZP:
		err = _LDA(self, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case LDA_ZX:
		err = _LDA(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem, c);
		break;
	case LDA_AB:
		err = _LDA(self, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;
	case LDA_AX:
		err = _LDA(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem, c);
		break;
	case LDA_AY:
		err = _LDA(self, NESEMU_ADDRESSING_ABSOLUTE_Y, mem, c);
		break;
	case LDA_IX:
		err = _LDA(self, NESEMU_ADDRESSING_INDIRECT_X, mem, c);
		break;
	case LDA_IY:
		err = _LDA(self, NESEMU_ADDRESSING_INDIRECT_Y, mem, c);
		break;

	// LDX
	case LDX_IM:
		err = _LDX(self, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case LDX_ZP:
		err = _LDX(self, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case LDX_ZY:
		err = _LDX(self, NESEMU_ADDRESSING_ZERO_PAGE_Y, mem, c);
		break;
	case LDX_AB:
		err = _LDX(self, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;
	case LDX_AY:
		err = _LDX(self, NESEMU_ADDRESSING_ABSOLUTE_Y, mem, c);
		break;

	// LDY
	case LDY_IM:
		err = _LDY(self, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case LDY_ZP:
		err = _LDY(self, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case LDY_ZX:
		err = _LDY(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem, c);
		break;
	case LDY_AB:
		err = _LDY(self, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;
	case LDY_AX:
		err = _LDY(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem, c);
		break;

	// STA
	case STA_ZP:
		err = _STA(self, NESEMU_ADDRESSING_ZERO_PAGE, mem);
		break;
	case STA_ZX:
		err = _STA(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem);
		break;
	case STA_AB:
		err = _STA(self, NESEMU_ADDRESSING_ABSOLUTE, mem);
		break;
	case STA_AX:
		err = _STA(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem);
		break;
	case STA_AY:
		err = _STA(self, NESEMU_ADDRESSING_ABSOLUTE_Y, mem);
		break;
	case STA_IX:
		err = _STA(self, NESEMU_ADDRESSING_INDIRECT_X, mem);
		break;
	case STA_IY:
		err = _STA(self, NESEMU_ADDRESSING_INDIRECT_Y, mem);
		break;

		// STX
	case STX_ZP:
		err = _STX(self, NESEMU_ADDRESSING_ZERO_PAGE, mem);
		break;
	case STX_ZY:
		err = _STX(self, NESEMU_ADDRESSING_ZERO_PAGE_Y, mem);
		break;
	case STX_AB:
		err = _STX(self, NESEMU_ADDRESSING_ABSOLUTE, mem);
		break;

	// STY
	case STY_ZP:
		err = _STY(self, NESEMU_ADDRESSING_ZERO_PAGE, mem);
		break;
	case STY_ZX:
		err = _STY(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem);
		break;
	case STY_AB:
		err = _STY(self, NESEMU_ADDRESSING_ABSOLUTE, mem);
		break;

		// TAX
	case TAX:
		_NESEMU_FALLTHROUGH;
	case TXA:
		_NESEMU_FALLTHROUGH;
	case TAY:
		_NESEMU_FALLTHROUGH;
	case TYA:
		_NESEMU_FALLTHROUGH;
	// $sp transfers
	case TSX:
		_NESEMU_FALLTHROUGH;
	case TXS:
		err = _TXX(self, opc);
		break;

		// AND
	case AND_IM:
		err = _AND(self, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case AND_ZP:
		err = _AND(self, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case AND_ZX:
		err = _AND(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem, c);
		break;
	case AND_AB:
		err = _AND(self, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;
	case AND_AX:
		err = _AND(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem, c);
		break;
	case AND_AY:
		err = _AND(self, NESEMU_ADDRESSING_ABSOLUTE_Y, mem, c);
		break;
	case AND_IX:
		err = _AND(self, NESEMU_ADDRESSING_INDIRECT_X, mem, c);
		break;
	case AND_IY:
		err = _AND(self, NESEMU_ADDRESSING_INDIRECT_Y, mem, c);
		break;

		// EOR
	case EOR_IM:
		err = _EOR(self, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case EOR_ZP:
		err = _EOR(self, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case EOR_ZX:
		err = _EOR(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem, c);
		break;
	case EOR_AB:
		err = _EOR(self, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;
	case EOR_AX:
		err = _EOR(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem, c);
		break;
	case EOR_AY:
		err = _EOR(self, NESEMU_ADDRESSING_ABSOLUTE_Y, mem, c);
		break;
	case EOR_IX:
		err = _EOR(self, NESEMU_ADDRESSING_INDIRECT_X, mem, c);
		break;
	case EOR_IY:
		err = _EOR(self, NESEMU_ADDRESSING_INDIRECT_Y, mem, c);
		break;

		// ORA
	case ORA_IM:
		err = _ORA(self, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case ORA_ZP:
		err = _ORA(self, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case ORA_ZX:
		err = _ORA(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem, c);
		break;
	case ORA_AB:
		err = _ORA(self, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;
	case ORA_AX:
		err = _ORA(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem, c);
		break;
	case ORA_AY:
		err = _ORA(self, NESEMU_ADDRESSING_ABSOLUTE_Y, mem, c);
		break;
	case ORA_IX:
		err = _ORA(self, NESEMU_ADDRESSING_INDIRECT_X, mem, c);
		break;
	case ORA_IY:
		err = _ORA(self, NESEMU_ADDRESSING_INDIRECT_Y, mem, c);
		break;

		// BIT
	case BIT_ZP:
		err = _BIT(self, NESEMU_ADDRESSING_ZERO_PAGE, mem);
		break;
	case BIT_AB:
		err = _BIT(self, NESEMU_ADDRESSING_ABSOLUTE, mem);
		break;

		// ADC
	case ADC_IM:
		err = _ADC(self, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case ADC_ZP:
		err = _ADC(self, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case ADC_ZX:
		err = _ADC(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem, c);
		break;
	case ADC_AB:
		err = _ADC(self, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;
	case ADC_AX:
		err = _ADC(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem, c);
		break;
	case ADC_AY:
		err = _ADC(self, NESEMU_ADDRESSING_ABSOLUTE_Y, mem, c);
		break;
	case ADC_IX:
		err = _ADC(self, NESEMU_ADDRESSING_INDIRECT_X, mem, c);
		break;
	case ADC_IY:
		err = _ADC(self, NESEMU_ADDRESSING_INDIRECT_Y, mem, c);
		break;

		// SBC
	case SBC_IM:
		err = _SBC(self, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case SBC_ZP:
		err = _SBC(self, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case SBC_ZX:
		err = _SBC(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem, c);
		break;
	case SBC_AB:
		err = _SBC(self, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;
	case SBC_AX:
		err = _SBC(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem, c);
		break;
	case SBC_AY:
		err = _SBC(self, NESEMU_ADDRESSING_ABSOLUTE_Y, mem, c);
		break;
	case SBC_IX:
		err = _SBC(self, NESEMU_ADDRESSING_INDIRECT_X, mem, c);
		break;
	case SBC_IY:
		err = _SBC(self, NESEMU_ADDRESSING_INDIRECT_Y, mem, c);
		break;

		// CMP
	case CMP_IM:
		err = _CMP(self, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case CMP_ZP:
		err = _CMP(self, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case CMP_ZX:
		err = _CMP(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem, c);
		break;
	case CMP_AB:
		err = _CMP(self, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;
	case CMP_AX:
		err = _CMP(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem, c);
		break;
	case CMP_AY:
		err = _CMP(self, NESEMU_ADDRESSING_ABSOLUTE_Y, mem, c);
		break;
	case CMP_IX:
		err = _CMP(self, NESEMU_ADDRESSING_INDIRECT_X, mem, c);
		break;
	case CMP_IY:
		err = _CMP(self, NESEMU_ADDRESSING_INDIRECT_Y, mem, c);
		break;

		// CPX
	case CPX_IM:
		err = _CPX(self, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case CPX_ZP:
		err = _CPX(self, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case CPX_AB:
		err = _CPX(self, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;

		// CPY
	case CPY_IM:
		err = _CPY(self, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case CPY_ZP:
		err = _CPY(self, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case CPY_AB:
		err = _CPY(self, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;

		// INC
	case INC_ZP:
		_INC(self, NESEMU_ADDRESSING_ZERO_PAGE, mem);
		break;
	case INC_ZX:
		_INC(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem);
		break;
	case INC_AB:
		_INC(self, NESEMU_ADDRESSING_ABSOLUTE, mem);
		break;
	case INC_AX:
		_INC(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem);
		break;

		// INX
	case INX:
		_INX(self);
		break;

		// INY
	case INY:
		_INY(self);
		break;

		// DEC
	case DEC_ZP:
		_DEC(self, NESEMU_ADDRESSING_ZERO_PAGE, mem);
		break;
	case DEC_ZX:
		_DEC(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem);
		break;
	case DEC_AB:
		_DEC(self, NESEMU_ADDRESSING_ABSOLUTE, mem);
		break;
	case DEC_AX:
		_DEC(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem);
		break;

		// DEX
	case DEX:
		_DEX(self);
		break;

		// DEY
	case DEY:
		_DEY(self);
		break;

		// ASL
	case ASL_ACC:
		_ASL(self, NESEMU_ADDRESSING_ACCUMULATOR, mem);
		break;
	case ASL_ZP:
		_ASL(self, NESEMU_ADDRESSING_ZERO_PAGE, mem);
		break;
	case ASL_ZX:
		_ASL(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem);
		break;
	case ASL_AB:
		_ASL(self, NESEMU_ADDRESSING_ABSOLUTE, mem);
		break;
	case ASL_AX:
		_ASL(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem);
		break;

		// LSR
	case LSR_ACC:
		_LSR(self, NESEMU_ADDRESSING_ACCUMULATOR, mem);
		break;
	case LSR_ZP:
		_LSR(self, NESEMU_ADDRESSING_ZERO_PAGE, mem);
		break;
	case LSR_ZX:
		_LSR(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem);
		break;
	case LSR_AB:
		_LSR(self, NESEMU_ADDRESSING_ABSOLUTE, mem);
		break;
	case LSR_AX:
		_LSR(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem);
		break;

		// ROL
	case ROL_ACC:
		_ROL(self, NESEMU_ADDRESSING_ACCUMULATOR, mem);
		break;
	case ROL_ZP:
		_ROL(self, NESEMU_ADDRESSING_ZERO_PAGE, mem);
		break;
	case ROL_ZX:
		_ROL(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem);
		break;
	case ROL_AB:
		_ROL(self, NESEMU_ADDRESSING_ABSOLUTE, mem);
		break;
	case ROL_AX:
		_ROL(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem);
		break;

		// ROR
	case ROR_ACC:
		_ROR(self, NESEMU_ADDRESSING_ACCUMULATOR, mem);
		break;
	case ROR_ZP:
		_ROR(self, NESEMU_ADDRESSING_ZERO_PAGE, mem);
		break;
	case ROR_ZX:
		_ROR(self, NESEMU_ADDRESSING_ZERO_PAGE_X, mem);
		break;
	case ROR_AB:
		_ROR(self, NESEMU_ADDRESSING_ABSOLUTE, mem);
		break;
	case ROR_AX:
		_ROR(self, NESEMU_ADDRESSING_ABSOLUTE_X, mem);
		break;

		/* Jumps & Calls */
	case JMP_AB:
        err = _JMP(self, mem, NESEMU_ADDRESSING_ABSOLUTE);
        break;

	case JMP_IX:
        err = _JMP(self, mem, NESEMU_ADDRESSING_INDIRECT_X);
        break;

        // JSR
	case JSR:
        err = _JSR(self, mem);
        break;
    
        // RTS
	case RTS:
        err = _RTS(self, mem);
        break;

		/* System Functions */
	case BRK:
        err = _BRK(self, mem);
        break;

        // RTI
	case RTI:
        err = _RTI(self, mem);
        break;

		/* Branches */
	case BCC:
		_BCC(self, mem, c);
		break;
	case BCS:
		_BCS(self, mem, c);
		break;
	case BEQ:
		_BEQ(self, mem, c);
		break;
	case BMI:
		_BMI(self, mem, c);
		break;
	case BNE:
		_BNE(self, mem, c);
		break;
	case BPL:
		_BPL(self, mem, c);
		break;
	case BVC:
		_BVC(self, mem, c);
		break;
	case BVS:
		_BVS(self, mem, c);
		break;

		/* Stack operations */
	case PHA:
		err = _PHA(self, mem);
		break;
	case PLA:
		err = _PLA(self, mem);
		break;
	case PHP:
		err = _PHP(self, mem);
		break;
	case PLP:
		err = _PLP(self, mem);
		break;

		/* Status Flag Changes */
	case CLC:
		_NESEMU_FALLTHROUGH;
	case CLD:
		_NESEMU_FALLTHROUGH;
	case CLI:
		_NESEMU_FALLTHROUGH;
	case CLV:
		_NESEMU_FALLTHROUGH;
	case SEC:
		_NESEMU_FALLTHROUGH;
	case SED:
		_NESEMU_FALLTHROUGH;
	case SEI:
		err = _CXX_SXX(self, opc);
		break;

		/* No operation */
	case NOP:
		break;

		/* Instruction not found */
	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	return err;
}
