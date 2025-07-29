/**
 * This file contains definitions for functions in 'cpu.h'
 * Contains code required to decode CPU instructions
 *
 *
 */

#include "nesemu/cpu/cpu.h"
#include "nesemu/cpu/instructions.h"
#include "nesemu/cpu/status.h"

#include "nesemu/memory/memory.h"
#include "nesemu/memory/paging.h"
#include "nesemu/memory/stack.h"

#include "nesemu/util/error.h"
#include "nesemu/util/bits.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Return error if not success
 */
#define RETURN_IF_ERROR(err)                \
	if (err != NESEMU_RETURN_SUCCESS) { \
		return err;                 \
	}

/* Private Functions */
static inline nesemu_error_t _LDA(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing addresses
	uint8_t lsb = 0, msb = 0;

	// Placeholder for a full address and pointer for indirect mode
	uint16_t addr = 0, ptr = 0;

	switch (opcode) {
	case LDA_IM:
		// Load Accumulator Immediate
		self->a = nes_cpu_fetch(self, mem);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		break;

	case LDA_ZP:
		// Load Accumulator Zero Page
		// Read contents at addr at zero page
		err = nes_mem_r8(
			mem, NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem)),
			&self->a);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		break;

	case LDA_ZX:
		// Load Accumulator Zero Page and add X register
		// Read contents at addr at zero page and add contents of X
		err = nes_mem_r8(mem,
				 NESEMU_ZEROPAGE_GET_ADDR(
					 nes_cpu_fetch(self, mem) + self->x),
				 &self->a);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		break;

	case LDA_AB:
		// Load Accumulator Absolute
		// Load contents at memory address
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		err = nes_mem_r8(mem, NESEMU_UTIL_U16(msb, lsb), &self->a);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		break;

	case LDA_AX:
		// Load Accumulator Absolute, X
		// Load contents at memory address and add X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb) + self->x;
		err = nes_mem_r8(mem, addr, &self->a);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		// Add one cycle if cross pagee
		if (nes_mem_is_crosspage(addr)) {
			*cycles += 1;
		}
		break;

	case LDA_AY:
		// Load Accumulator Absolute, Y
		// Load contents at memory address and add Y
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb) + self->y;
		err = nes_mem_r8(mem, addr, &self->a);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		// Add one cycle if cross pagee
		if (nes_mem_is_crosspage(addr)) {
			*cycles += 1;
		}
		break;

	case LDA_IX:
		// Load Accumulator, Indirect X (Pre-Indexed)
		// Load the pointer address (always zero page) and add X
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
					       self->x);
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Read contents from the actual address into acc register
		err = nes_mem_r8(mem, addr, &self->a);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		break;

	case LDA_IY:
		// Load Accumulator, Indirect Y (Post-Indexed)
		// Load the pointer address (always zero page)
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Add Y to the actual address
		addr += self->y;
		// Read contents from the actual address into acc register
		err = nes_mem_r8(mem, addr, &self->a);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		break;
	}

	return err;
}

static inline nesemu_error_t _LDX(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing addresses
	uint8_t lsb = 0, msb = 0;

	// Placeholder for a full address and pointer for indirect mode
	uint16_t addr = 0, ptr = 0;

	switch (opcode) {
	case LDX_IM:
		// Load X Immediate
		self->x = nes_cpu_fetch(self, mem);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->x));
		break;

	case LDX_ZP:
		// Load X Zero Page
		// Read contents at addr at zero page
		err = nes_mem_r8(
			mem, NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem)),
			&self->x);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->x));
		break;

	case LDX_ZY:
		// Load Y Zero Page and add Y reegistere
		// Read contents at addr at zero page
		err = nes_mem_r8(mem,
				 NESEMU_ZEROPAGE_GET_ADDR(
					 nes_cpu_fetch(self, mem) + self->y),
				 &self->x);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->x));
		break;

	case LDX_AB:
		// Load X Absolute
		// Load contents at memory address
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		err = nes_mem_r8(mem, NESEMU_UTIL_U16(msb, lsb), &self->x);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->x));
		break;

	case LDX_AY:
		// Load X Absolute, Y
		// Load contents at memory address and add Y
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb) + self->y;
		err = nes_mem_r8(mem, addr, &self->x);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->x));
		// Add one cycle if cross pagee
		if (nes_mem_is_crosspage(addr)) {
			*cycles += 1;
		}
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	return err;
}

static inline nesemu_error_t _LDY(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing addresses
	uint8_t lsb = 0, msb = 0;

	// Placeholder for a full address and pointer for indirect mode
	uint16_t addr = 0, ptr = 0;

	switch (opcode) {
	case LDY_IM:
		// Load Y Immediate
		self->y = nes_cpu_fetch(self, mem);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->y));
		break;

	case LDY_ZP:
		// Load Y Zero Page
		// Read contents at addr at zero page
		err = nes_mem_r8(
			mem, NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem)),
			&self->y);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->y));
		break;

	case LDY_ZX:
		// Load X Zero Page and add X register
		// Read contents at addr at zero page
		err = nes_mem_r8(mem,
				 NESEMU_ZEROPAGE_GET_ADDR(
					 nes_cpu_fetch(self, mem) + self->x),
				 &self->y);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->y));
		break;

	case LDY_AB:
		// Load Y Absolute
		// Load contents at memory address
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		err = nes_mem_r8(mem, NESEMU_UTIL_U16(msb, lsb), &self->y);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->y));
		break;

	case LDY_AX:
		// Load Y Absolute, X
		// Load contents at memory address and add X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb) + self->x;
		err = nes_mem_r8(mem, addr, &self->y);
		RETURN_IF_ERROR(err);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->y));
		// Add one cycle if cross pagee
		if (nes_mem_is_crosspage(addr)) {
			*cycles += 1;
		}
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	return err;
}

static inline nesemu_error_t _STA(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing addresses
	uint8_t lsb = 0, msb = 0;

	// Placeholder for a full address and pointer for indirect mode
	uint16_t addr = 0, ptr = 0;

	switch (opcode) {
	case STA_ZP:
		// Store Accumulator, Zero Page
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		err = nes_mem_w8(mem, addr, self->a);
		break;

	case STA_ZX:
		// Store Accumulator, Zero Page + X
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem)) +
		       self->x;
		err = nes_mem_w8(mem, addr, self->a);
		break;

	case STA_AB:
		// Store Accumulator, Absolute
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb);
		err = nes_mem_w8(mem, addr, self->a);
		break;

	case STA_AX:
		// Store Accumulator, Absolute + X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb) + self->x;
		err = nes_mem_w8(mem, addr, self->a);
		break;

	case STA_AY:
		// Store Accumulator, Absolute + Y
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb) + self->y;
		err = nes_mem_w8(mem, addr, self->a);
		break;

	case STA_IX:
		// Store Accumulator, Indirect X (Pre-Indexed)
		// Load the pointer address (always zero page) and add X
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
					       self->x);
		// Contents in ptr address will be the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Write $acc into actual address
		err = nes_mem_w8(mem, addr, self->a);
		break;

	case STA_IY:
		// Store Accumulator, Indirect Y (Post-Indexed)
		// Load the pointer address (always zero page)
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		// Contents in ptr address will be the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Add Y to the actual address
		addr += self->y;
		// Write $acc into actual address
		err = nes_mem_w8(mem, addr, self->a);
		break;
	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	return err;
}

static inline nesemu_error_t _STX(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing addresses
	uint8_t lsb = 0, msb = 0;

	// Placeholder for a full address and pointer for indirect mode
	uint16_t addr = 0, ptr = 0;

	switch (opcode) {
	case STX_ZP:
		// Store X, Zero Page
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		err = nes_mem_w8(mem, addr, self->x);
		break;

	case STX_ZY:
		// Store X, Zero Page + Y
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem)) +
		       self->y;
		err = nes_mem_w8(mem, addr, self->x);
		break;

	case STX_AB:
		// Store X, Absolute
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb);
		err = nes_mem_w8(mem, addr, self->x);
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	return err;
}

static inline nesemu_error_t _STY(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing addresses
	uint8_t lsb = 0, msb = 0;

	// Placeholder for a full address and pointer for indirect mode
	uint16_t addr = 0, ptr = 0;

	switch (opcode) {
	case STY_ZP:
		// Store Y, Zero Page
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		err = nes_mem_w8(mem, addr, self->y);
		break;

	case STY_ZX:
		// Store Y, Zero Page + X
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem)) +
		       self->x;
		err = nes_mem_w8(mem, addr, self->y);
		break;

	case STY_AB:
		// Store Y, Absolute
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb);
		err = nes_mem_w8(mem, addr, self->y);
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	return err;
}

static inline nesemu_error_t _TXX(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	switch (opcode) {
	case TAX:
		// Transfer Accumulator to Index X
		self->x = self->a;
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->x));
		break;

	case TXA:
		// Transfer X to A
		self->a = self->x;
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		break;

	case TAY:
		// Transfer A to Y
		self->y = self->a;
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->y));
		break;

	case TYA:
		// Transfer Y to A
		self->a = self->y;
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	return err;
}

static inline nesemu_error_t _PXX(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholder
	uint8_t p = 0;

	switch (opcode) {
	case TSX:
		// Transfer $sp to $x
		self->x = self->sp;
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->x));
		break;

	case TXS:
		// Transfer $x to $sp
		self->sp = self->x;
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->sp));
		break;

	case PHA:
		// Push $a into stack
		err = nes_stack_push(mem, &self->sp, self->a);
		RETURN_IF_ERROR(err);
		break;

	case PLA:
		// Pull stack into $a
		err = nes_stack_pop(mem, &self->sp, &self->a);
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->sp));
		RETURN_IF_ERROR(err);
		break;

	case PHP:
		// Push processor status
		err = nes_stack_push(mem, &self->sp,
				     self->status | NESEMU_CPU_FLAGS_B);
		RETURN_IF_ERROR(err);
		break;

	case PLP:
		// Pull processor status
		err = nes_stack_pop(mem, &self->sp, &p);
		RETURN_IF_ERROR(err);
		self->status = p & ~(NESEMU_CPU_FLAGS_B | NESEMU_CPU_FLAGS_1);
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	return err;
}

static inline nesemu_error_t _AND(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing results
	uint8_t lsb = 0, msb = 0, p = 0;

	// Placeholder for a full address AND pointer for indirect mode
	uint16_t addr = 0, ptr = 0;

	switch (opcode) {
	case AND_IM:
		// Bitwise AND, Immediate
		p = nes_cpu_fetch(self, mem);
		break;

	case AND_ZP:
		// Bitwise AND, Zero Page
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		err = nes_mem_r8(mem, addr, &p);
		break;

	case AND_ZX:
		// Bitwise AND, Zero Page + X
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem)) +
		       self->x;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case AND_AB:
		// Bitwise AND, Absolute
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb);
		err = nes_mem_r8(mem, addr, &p);
		break;

	case AND_AX:
		// Bitwise AND, Absolute X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		// Add x to address
		addr = NESEMU_UTIL_U16(msb, lsb) + self->x;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case AND_AY:
		// Bitwise AND, Absolute X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		// Add y to address
		addr = NESEMU_UTIL_U16(msb, lsb) + self->y;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case AND_IX:
		// Bitwise AND, Indirect X (Pre-Indexed)
		// Load the pointer address (zero page) AND add X
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
					       self->x);
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Read contents from the actual address
		err = nes_mem_r8(mem, addr, &p);
		break;

	case AND_IY:
		// Bitwise AND, Indirect Y (Pre-Indexed)
		// Load the pointer address (zero page)
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Add Y to the actual address
		addr += self->y;
		// Read contents from the actual address
		err = nes_mem_r8(mem, addr, &p);
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	// Use 'p' as the value to apply to $a
	RETURN_IF_ERROR(err);
	self->a &= p;
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->a));

	return err;
}

static inline nesemu_error_t _EOR(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing results
	uint8_t lsb = 0, msb = 0, p = 0;

	// Placeholder for a full address EOR pointer for indirect mode
	uint16_t addr = 0, ptr = 0;

	switch (opcode) {
	case EOR_IM:
		// Bitwise EOR, Immediate
		p = nes_cpu_fetch(self, mem);
		break;

	case EOR_ZP:
		// Bitwise EOR, Zero Page
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		err = nes_mem_r8(mem, addr, &p);
		break;

	case EOR_ZX:
		// Bitwise EOR, Zero Page + X
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem)) +
		       self->x;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case EOR_AB:
		// Bitwise EOR, Absolute
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb);
		err = nes_mem_r8(mem, addr, &p);
		break;

	case EOR_AX:
		// Bitwise EOR, Absolute X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		// Add x to address
		addr = NESEMU_UTIL_U16(msb, lsb) + self->x;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case EOR_AY:
		// Bitwise EOR, Absolute X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		// Add y to address
		addr = NESEMU_UTIL_U16(msb, lsb) + self->y;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case EOR_IX:
		// Bitwise EOR, Indirect X (Pre-Indexed)
		// Load the pointer address (zero page) EOR add X
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
					       self->x);
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Read contents from the actual address
		err = nes_mem_r8(mem, addr, &p);
		break;

	case EOR_IY:
		// Bitwise EOR, Indirect Y (Pre-Indexed)
		// Load the pointer address (zero page)
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Add Y to the actual address
		addr += self->y;
		// Read contents from the actual address
		err = nes_mem_r8(mem, addr, &p);
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	// Use 'p' as the value to apply to $a
	RETURN_IF_ERROR(err);
	self->a ^= p;
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->a));

	return err;
}

static inline nesemu_error_t _ORA(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing results
	uint8_t lsb = 0, msb = 0, p = 0;

	// Placeholder for a full address ORA pointer for indirect mode
	uint16_t addr = 0, ptr = 0;

	switch (opcode) {
	case ORA_IM:
		// Bitwise ORA, Immediate
		p = nes_cpu_fetch(self, mem);
		break;

	case ORA_ZP:
		// Bitwise ORA, Zero Page
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		err = nes_mem_r8(mem, addr, &p);
		break;

	case ORA_ZX:
		// Bitwise ORA, Zero Page + X
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem)) +
		       self->x;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case ORA_AB:
		// Bitwise ORA, Absolute
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb);
		err = nes_mem_r8(mem, addr, &p);
		break;

	case ORA_AX:
		// Bitwise ORA, Absolute X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		// Add x to address
		addr = NESEMU_UTIL_U16(msb, lsb) + self->x;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case ORA_AY:
		// Bitwise ORA, Absolute X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		// Add y to address
		addr = NESEMU_UTIL_U16(msb, lsb) + self->y;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case ORA_IX:
		// Bitwise ORA, Indirect X (Pre-Indexed)
		// Load the pointer address (zero page) ORA add X
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
					       self->x);
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Read contents from the actual address
		err = nes_mem_r8(mem, addr, &p);
		break;

	case ORA_IY:
		// Bitwise ORA, Indirect Y (Pre-Indexed)
		// Load the pointer address (zero page)
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Add Y to the actual address
		addr += self->y;
		// Read contents from the actual address
		err = nes_mem_r8(mem, addr, &p);
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	// Use 'p' as the value to apply to $a
	RETURN_IF_ERROR(err);
	self->a |= p;
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->a));

	return err;
}

static inline nesemu_error_t _BIT(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing results
	uint8_t lsb = 0, msb = 0, p = 0;

	// Placeholder for a full address
	uint16_t addr = 0;

	switch (opcode) {
	case BIT_ZP:
		// Bitwise BIT, Zero Page
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		err = nes_mem_r8(mem, addr, &p);
		// This code only loads contents into <p>
		// Logic is after the switch block
		break;

	case BIT_AB:
		// Bitwise BIT, Absolute
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb);
		err = nes_mem_r8(mem, addr, &p);
		// This code only loads contents into <p>
		// Logic is after the switch block
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	// At this point, value <p> should be set
	RETURN_IF_ERROR(err);

	// Z if p is 0, V and N flags are the same as in p
	self->status |= NESEMU_CPU_STATUS_MASK_Z(p);
	self->status |= (p & NESEMU_CPU_FLAGS_V);
	self->status |= (p & NESEMU_CPU_FLAGS_N);

	return err;
}

static inline nesemu_error_t _ADC(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing results
	uint8_t lsb = 0, msb = 0, p = 0;

	// Placeholder for a full address ADC pointer for indirect mode
	uint16_t addr = 0, ptr = 0;

	switch (opcode) {
	case ADC_IM:
		// ADC, Immediate
		p = nes_cpu_fetch(self, mem);
		break;

	case ADC_ZP:
		// ADC, Zero Page
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		err = nes_mem_r8(mem, addr, &p);
		break;

	case ADC_ZX:
		// ADC, Zero Page + X
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem)) +
		       self->x;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case ADC_AB:
		// ADC, Absolute
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb);
		err = nes_mem_r8(mem, addr, &p);
		break;

	case ADC_AX:
		// ADC, Absolute X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		// Add x to address
		addr = NESEMU_UTIL_U16(msb, lsb) + self->x;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case ADC_AY:
		// ADC, Absolute X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		// Add y to address
		addr = NESEMU_UTIL_U16(msb, lsb) + self->y;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case ADC_IX:
		// ADC, Indirect X (Pre-Indexed)
		// Load the pointer address (zero page) ADC add X
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
					       self->x);
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Read contents from the actual address
		err = nes_mem_r8(mem, addr, &p);
		break;

	case ADC_IY:
		// ADC, Indirect Y (Pre-Indexed)
		// Load the pointer address (zero page)
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Add Y to the actual address
		addr += self->y;
		// Read contents from the actual address
		err = nes_mem_r8(mem, addr, &p);
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	// Use 'p' as the value to apply to $a
	RETURN_IF_ERROR(err);
	uint16_t result = self->a + p + (self->status & NESEMU_CPU_FLAGS_C);

	// Clear carry
	self->status =
		NESEMU_CPU_STATUS_UNSET_MASK(self->status, NESEMU_CPU_FLAGS_C);

	// Set status
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(result));

	// Set carry (result overflow)
	if (result > UINT8_MAX) {
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C);
	}

	// Overflow
	if (((uint8_t)result ^ self->a) & ((uint8_t)result ^ p) &
	    NESEMU_CPU_FLAGS_N) {
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_V);
	}

	// Set the value
	self->a = (uint8_t)result;
	return err;
}

static inline nesemu_error_t _SBC(struct nes_cpu_t *self,
				  uint8_t opcode,
				  nes_memory_t mem,
				  int *cycles)
{
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Placeholders for storing results
	uint8_t lsb = 0, msb = 0, p = 0;

	// Placeholder for a full address SBC pointer for indirect mode
	uint16_t addr = 0, ptr = 0;

	switch (opcode) {
	case SBC_IM:
		// SBC, Immediate
		p = nes_cpu_fetch(self, mem);
		break;

	case SBC_ZP:
		// SBC, Zero Page
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		err = nes_mem_r8(mem, addr, &p);
		break;

	case SBC_ZX:
		// SBC, Zero Page + X
		addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem)) +
		       self->x;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case SBC_AB:
		// SBC, Absolute
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		addr = NESEMU_UTIL_U16(msb, lsb);
		err = nes_mem_r8(mem, addr, &p);
		break;

	case SBC_AX:
		// SBC, Absolute X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		// Add x to address
		addr = NESEMU_UTIL_U16(msb, lsb) + self->x;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case SBC_AY:
		// SBC, Absolute X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		// Add y to address
		addr = NESEMU_UTIL_U16(msb, lsb) + self->y;
		err = nes_mem_r8(mem, addr, &p);
		break;

	case SBC_IX:
		// SBC, Indirect X (Pre-Indexed)
		// Load the pointer address (zero page) SBC add X
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
					       self->x);
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Read contents from the actual address
		err = nes_mem_r8(mem, addr, &p);
		break;

	case SBC_IY:
		// SBC, Indirect Y (Pre-Indexed)
		// Load the pointer address (zero page)
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		RETURN_IF_ERROR(err);
		// Add Y to the actual address
		addr += self->y;
		// Read contents from the actual address
		err = nes_mem_r8(mem, addr, &p);
		break;

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	// Use 'p' as the value to apply to $a
	RETURN_IF_ERROR(err);
	uint16_t result = self->a - p - ~(self->status & NESEMU_CPU_FLAGS_C);

	// Clear carry
	self->status =
		NESEMU_CPU_STATUS_UNSET_MASK(self->status, NESEMU_CPU_FLAGS_C);

	// Set status
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(result));

	// Set carry (result underflow < 0)
	if (result > UINT8_MAX) {
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_C);
	}

	// Overflow
	if (((uint8_t)result ^ self->a) & ((uint8_t)result ^ ~p) & NESEMU_CPU_FLAGS_N) {
		nes_cpu_status_mask_set(self, NESEMU_CPU_FLAGS_V);
	}

	// Set the value
	self->a = (uint8_t)result;
	return err;
}

/* Public Functions */

nesemu_error_t nes_cpu_next(struct nes_cpu_t *self,
			    nes_memory_t mem,
			    int *cycles)
{
	/* Check input arguments */
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (cycles == NULL) {
	}
#endif
	// Error code
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Get instruction opcode
	uint8_t opcode = nes_cpu_fetch(self, mem);

	// Set cycles using the instruction
	*cycles = nes_cpu_op_cycles[opcode];

	// Placeholders for storing addresses
	uint8_t lsb = 0, msb = 0;

	// Placeholder for a full address and pointer for indirect mode
	uint16_t addr = 0, ptr = 0;

	// Decode the instruction
	switch (opcode) {
		/* Load/Store Operations */
	case LDA_IM:
	case LDA_ZP:
	case LDA_ZX:
	case LDA_AB:
	case LDA_AX:
	case LDA_AY:
	case LDA_IX:
	case LDA_IY:
		err = _LDA(self, opcode, mem, cycles);
		break;

	case LDX_IM:
	case LDX_ZP:
	case LDX_ZY:
	case LDX_AB:
	case LDX_AY:
		err = _LDX(self, opcode, mem, cycles);
		break;

	case LDY_IM:
	case LDY_ZP:
	case LDY_ZX:
	case LDY_AB:
	case LDY_AX:
		err = _LDY(self, opcode, mem, cycles);
		break;

	case STA_ZP:
	case STA_ZX:
	case STA_AB:
	case STA_AX:
	case STA_AY:
	case STA_IX:
	case STA_IY:
		err = _STA(self, opcode, mem, cycles);
		break;

	case STX_ZP:
	case STX_ZY:
	case STX_AB:
		err = _STX(self, opcode, mem, cycles);
		break;

	case STY_ZP:
	case STY_ZX:
	case STY_AB:
		err = _STY(self, opcode, mem, cycles);
		break;

		/* Register Transfers */
	case TAX:
	case TXA:
	case TAY:
	case TYA:
		err = _TXX(self, opcode, mem, cycles);
		break;

		/* Stack operations */
	case TSX:
	case TXS:
	case PHA:
	case PLA:
	case PHP:
	case PLP:
		err = _PXX(self, opcode, mem, cycles);
		break;

		/* Logical */
	case AND_IM:
	case AND_ZP:
	case AND_ZX:
	case AND_AB:
	case AND_AX:
	case AND_AY:
	case AND_IX:
	case AND_IY:
		err = _AND(self, opcode, mem, cycles);
		break;

	case EOR_IM:
	case EOR_ZP:
	case EOR_ZX:
	case EOR_AB:
	case EOR_AX:
	case EOR_AY:
	case EOR_IX:
	case EOR_IY:
		err = _EOR(self, opcode, mem, cycles);
		break;

	case ORA_IM:
	case ORA_ZP:
	case ORA_ZX:
	case ORA_AB:
	case ORA_AX:
	case ORA_AY:
	case ORA_IX:
	case ORA_IY:
		err = _ORA(self, opcode, mem, cycles);
		break;

	case BIT_ZP:
	case BIT_AB:
		err = _BIT(self, opcode, mem, cycles);
		break;

		/* Arithmetic */
	case ADC_IM:
	case ADC_ZP:
	case ADC_ZX:
	case ADC_AB:
	case ADC_AX:
	case ADC_AY:
	case ADC_IX:
	case ADC_IY:
		err = _ADC(self, opcode, mem, cycles);
		break;

	case SBC_IM:
	case SBC_ZP:
	case SBC_ZX:
	case SBC_AB:
	case SBC_AX:
	case SBC_AY:
	case SBC_IX:
	case SBC_IY:
        err = _SBC(self, opcode, mem, cycles);
        break;

	case CMP_IM:
	case CMP_ZP:
	case CMP_ZX:
	case CMP_AB:
	case CMP_AX:
	case CMP_AY:
	case CMP_IX:
	case CMP_IY:

	case CPX_IM:
	case CPX_ZP:
	case CPX_AB:

	case CPY_IM:
	case CPY_ZP:
	case CPY_AB:

		/* Increments & Decrements */
	case INC_ZP:
	case INC_ZX:
	case INC_AB:
	case INC_AX:

	case INX:
	case INY:

	case DEC_ZP:
	case DEC_ZX:
	case DEC_AB:
	case DEC_AX:

	case DEX:
	case DEY:

		/* Shifts */
	case ASL_ACC:
	case ASL_ZP:
	case ASL_ZX:
	case ASL_AB:
	case ASL_AX:

	case LSR_ACC:
	case LSR_ZP:
	case LSR_ZX:
	case LSR_AB:
	case LSR_AX:

	case ROL_ACC:
	case ROL_ZP:
	case ROL_ZX:
	case ROL_AB:
	case ROL_AX:

	case ROR_ACC:
	case ROR_ZP:
	case ROR_ZX:
	case ROR_AB:
	case ROR_AX:

		/* Jumps & Calls */
	case JMP_AB:
	case JMP_IX:

	case JSR:
	case RTS:

		/* Branches */
	case BCC:
	case BCS:
	case BEQ:
	case BMI:
	case BNE:
	case BPL:
	case BVC:
	case BVS:

		/* Status Flag Changes */
	case CLC:
	case CLD:
	case CLI:
	case CLV:
	case SEC:
	case SED:
	case SEI:

		/* System Functions */
	case BRK:
	case NOP:
	case RTI:

		/* Instruction not found */
	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	return err;
}
