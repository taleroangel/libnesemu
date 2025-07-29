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

/* Private Functions */

/**
 * Get a u8 memory value given the addressing mode
 * @note NESEMU_ADDRESSING_IMMEDIATE not supported! will return error
 * @param addr Reference to where the memory address to be used is stored.
 */
static nesemu_error_t get_addr(struct nes_cpu_t *self,
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
	case NESEMU_ADDRESSING_UNSPECIFIED:
		__attribute__((fallthrough));
	case NESEMU_ADDRESSING_IMMEDIATE:
		return NESEMU_RETURN_CPU_BAD_ADDRESSING;

	case NESEMU_ADDRESSING_ZERO_PAGE:
		// Load Accumulator Zero Page
		*addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		break;

	case NESEMU_ADDRESSING_ZERO_PAGE_X:
		// Load Accumulator Zero Page and add X register to addr
		*addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
						 self->x);
		break;

	case NESEMU_ADDRESSING_ZERO_PAGE_Y:
		// Load Accumulator Zero Page and add Y register to addr
		*addr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
						 self->y);
		break;

	case NESEMU_ADDRESSING_ABSOLUTE:
		// Load Accumulator Absolute
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		*addr = NESEMU_UTIL_U16(msb, lsb);
		break;

	case NESEMU_ADDRESSING_ABSOLUTE_X:
		// Load Accumulator Absolute, X
		// Load contents at memory address and add X
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		*addr = NESEMU_UTIL_U16(msb, lsb) + self->x;
		break;

	case NESEMU_ADDRESSING_ABSOLUTE_Y:
		// Load Accumulator Absolute, Y
		// Load contents at memory address and add Y
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		*addr = NESEMU_UTIL_U16(msb, lsb) + self->y;
		break;

	case NESEMU_ADDRESSING_INDIRECT:
		// Load Accumulator, Indirect X (Pre-Indexed)
		// Load the pointer address (always zero page)
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, addr);
		break;

	case NESEMU_ADDRESSING_INDIRECT_X:
		// Load Accumulator, Indirect X (Pre-Indexed)
		// Load the pointer address (always zero page) and add X
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem) +
					       self->x);
		// Contents in ptr will be used as the actual address
		err = nes_mem_r16(mem, ptr, addr);
		break;

	case NESEMU_ADDRESSING_INDIRECT_Y:
		// Load Accumulator, Indirect Y (Post-Indexed)
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

static nesemu_error_t _LDA(struct nes_cpu_t *self,
			   uint8_t opcode,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	switch (addressing) {
	// Immediate mode not supported
	case NESEMU_ADDRESSING_IMMEDIATE:
		self->a = nes_cpu_fetch(self, mem);
		break;

	// Additional cycle if page crossed
	case NESEMU_ADDRESSING_ABSOLUTE_X:
	case NESEMU_ADDRESSING_ABSOLUTE_Y:
	case NESEMU_ADDRESSING_INDIRECT_Y:
		if (nes_mem_is_crosspage(addr)) {
			*cycles += 1;
		}
		__attribute__((fallthrough));
	default:
		// Get the address
		err = get_addr(self, addressing, mem, &addr);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}

		// Read value
		err = nes_mem_r8(mem, addr, &self->a);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;
	}

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->a));

	return err;
}

static nesemu_error_t _LDX(struct nes_cpu_t *self,
			   uint8_t opcode,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	switch (addressing) {
	// Immediate mode not supported
	case NESEMU_ADDRESSING_IMMEDIATE:
		self->x = nes_cpu_fetch(self, mem);
		break;

	// Additional cycle if page crossed
	case NESEMU_ADDRESSING_ABSOLUTE_Y:
		if (nes_mem_is_crosspage(addr)) {
			*cycles += 1;
		}
		__attribute__((fallthrough));
	default:
		// Get the address
		err = get_addr(self, addressing, mem, &addr);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}

		// Read value
		err = nes_mem_r8(mem, addr, &self->x);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;
	}

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->x));

	return err;
}

static nesemu_error_t _LDY(struct nes_cpu_t *self,
			   uint8_t opcode,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	switch (addressing) {
	// Immediate mode not supported
	case NESEMU_ADDRESSING_IMMEDIATE:
		self->y = nes_cpu_fetch(self, mem);
		break;

	// Additional cycle if page crossed
	case NESEMU_ADDRESSING_ABSOLUTE_X:
		if (nes_mem_is_crosspage(addr)) {
			*cycles += 1;
		}
		__attribute__((fallthrough));
	default:
		// Get the address
		err = get_addr(self, addressing, mem, &addr);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}

		// Read value
		err = nes_mem_r8(mem, addr, &self->y);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		break;
	}

	// Update status flags
	nes_cpu_status_mask_set(self, NESEMU_CPU_STATUS_MASK_NZ(self->y));

	return err;
}

static nesemu_error_t _STA(struct nes_cpu_t *self,
			   uint8_t opcode,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Get the address
	err = get_addr(self, addressing, mem, &addr);
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
			   uint8_t opcode,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Get the address
	err = get_addr(self, addressing, mem, &addr);
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
			   uint8_t opcode,
			   enum nes_cpu_addressing_mode_t addressing,
			   nes_memory_t mem,
			   int *cycles)
{
	// Decode the value
	nesemu_error_t err = NESEMU_RETURN_SUCCESS;

	// Where to store the memory address that is going to be accesed
	uint16_t addr = 0;

	// Get the address
	err = get_addr(self, addressing, mem, &addr);
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

static nesemu_error_t _TXX(struct nes_cpu_t *self,
			   uint8_t opcode,
			   nes_memory_t mem,
			   int *cycles)
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

	default:
		return NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION;
	}

	return NESEMU_RETURN_SUCCESS;
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
		/* Load/Store Operations */

		// LDA
	case LDA_IM:
		err = _LDA(self, opc, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case LDA_ZP:
		err = _LDA(self, opc, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case LDA_ZX:
		err = _LDA(self, opc, NESEMU_ADDRESSING_ZERO_PAGE_X, mem, c);
		break;
	case LDA_AB:
		err = _LDA(self, opc, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;
	case LDA_AX:
		err = _LDA(self, opc, NESEMU_ADDRESSING_ABSOLUTE_X, mem, c);
		break;
	case LDA_AY:
		err = _LDA(self, opc, NESEMU_ADDRESSING_ABSOLUTE_Y, mem, c);
		break;
	case LDA_IX:
		err = _LDA(self, opc, NESEMU_ADDRESSING_INDIRECT_X, mem, c);
		break;
	case LDA_IY:
		err = _LDA(self, opc, NESEMU_ADDRESSING_INDIRECT_Y, mem, c);
		break;

		// LDX
	case LDX_IM:
		err = _LDX(self, opc, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case LDX_ZP:
		err = _LDX(self, opc, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case LDX_ZY:
		err = _LDX(self, opc, NESEMU_ADDRESSING_ZERO_PAGE_Y, mem, c);
		break;
	case LDX_AB:
		err = _LDX(self, opc, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;
	case LDX_AY:
		err = _LDX(self, opc, NESEMU_ADDRESSING_ABSOLUTE_Y, mem, c);
		break;

	// LDY
	case LDY_IM:
		err = _LDY(self, opc, NESEMU_ADDRESSING_IMMEDIATE, mem, c);
		break;
	case LDY_ZP:
		err = _LDY(self, opc, NESEMU_ADDRESSING_ZERO_PAGE, mem, c);
		break;
	case LDY_ZX:
		err = _LDY(self, opc, NESEMU_ADDRESSING_ZERO_PAGE_X, mem, c);
		break;
	case LDY_AB:
		err = _LDY(self, opc, NESEMU_ADDRESSING_ABSOLUTE, mem, c);
		break;
	case LDY_AX:
		err = _LDY(self, opc, NESEMU_ADDRESSING_ABSOLUTE_X, mem, c);
		break;

		// STA
	case STA_ZP:
		err = _STA(self, opc, REPLACE_ZP, mem, c);
		break;
	case STA_ZX:
		err = _STA(self, opc, REPLACE_ZX, mem, c);
		break;
	case STA_AB:
		err = _STA(self, opc, REPLACE_AB, mem, c);
		break;
	case STA_AX:
		err = _STA(self, opc, REPLACE_AX, mem, c);
		break;
	case STA_AY:
		err = _STA(self, opc, REPLACE_AY, mem, c);
		break;
	case STA_IX:
		err = _STA(self, opc, REPLACE_IX, mem, c);
		break;
	case STA_IY:
		err = _STA(self, opc, REPLACE_IY, mem, c);
		break;

		// STX
	case STX_ZP:
		err = _STX(self, opc, REPLACE_ZP, mem, c);
		break;
	case STX_ZY:
		err = _STX(self, opc, REPLACE_ZY, mem, c);
		break;
	case STX_AB:
		err = _STX(self, opc, REPLACE_AB, mem, c);
		break;

	// STY
	case STY_ZP:
		err = _STY(self, opc, REPLACE_ZP, mem, c);
		break;
	case STY_ZX:
		err = _STY(self, opc, REPLACE_ZX, mem, c);
		break;
	case STY_AB:
		err = _STY(self, opc, REPLACE_AB, mem, c);
		break;

		/* Register Transfers */
	case TAX:
		__attribute__((fallthrough));
	case TXA:
		__attribute__((fallthrough));
	case TAY:
		__attribute__((fallthrough));
	case TYA:
		err = _TXX(self, opc, mem, c);
		break;

		/* Stack operations */
	case TSX:
	case TXS:
	case PHA:
	case PLA:
	case PHP:
	case PLP:

		/* Logical */
	case AND_IM:
		err = _AND(self, opc, REPLACE_IM, mem, c);
		break;
	case AND_ZP:
		err = _AND(self, opc, REPLACE_ZP, mem, c);
		break;
	case AND_ZX:
		err = _AND(self, opc, REPLACE_ZX, mem, c);
		break;
	case AND_AB:
		err = _AND(self, opc, REPLACE_AB, mem, c);
		break;
	case AND_AX:
		err = _AND(self, opc, REPLACE_AX, mem, c);
		break;
	case AND_AY:
		err = _AND(self, opc, REPLACE_AY, mem, c);
		break;
	case AND_IX:
		err = _AND(self, opc, REPLACE_IX, mem, c);
		break;
	case AND_IY:
		err = _AND(self, opc, REPLACE_IY, mem, c);
		break;

	case EOR_IM:
		err = _EOR(self, opc, REPLACE_IM, mem, c);
		break;
	case EOR_ZP:
		err = _EOR(self, opc, REPLACE_ZP, mem, c);
		break;
	case EOR_ZX:
		err = _EOR(self, opc, REPLACE_ZX, mem, c);
		break;
	case EOR_AB:
		err = _EOR(self, opc, REPLACE_AB, mem, c);
		break;
	case EOR_AX:
		err = _EOR(self, opc, REPLACE_AX, mem, c);
		break;
	case EOR_AY:
		err = _EOR(self, opc, REPLACE_AY, mem, c);
		break;
	case EOR_IX:
		err = _EOR(self, opc, REPLACE_IX, mem, c);
		break;
	case EOR_IY:
		err = _EOR(self, opc, REPLACE_IY, mem, c);
		break;

	case ORA_IM:
		err = _ORA(self, opc, REPLACE_IM, mem, c);
		break;
	case ORA_ZP:
		err = _ORA(self, opc, REPLACE_ZP, mem, c);
		break;
	case ORA_ZX:
		err = _ORA(self, opc, REPLACE_ZX, mem, c);
		break;
	case ORA_AB:
		err = _ORA(self, opc, REPLACE_AB, mem, c);
		break;
	case ORA_AX:
		err = _ORA(self, opc, REPLACE_AX, mem, c);
		break;
	case ORA_AY:
		err = _ORA(self, opc, REPLACE_AY, mem, c);
		break;
	case ORA_IX:
		err = _ORA(self, opc, REPLACE_IX, mem, c);
		break;
	case ORA_IY:
		err = _ORA(self, opc, REPLACE_IY, mem, c);
		break;

	case BIT_ZP:
		err = _BIT(self, opc, REPLACE_ZP, mem, c);
		break;
	case BIT_AB:
		err = _BIT(self, opc, REPLACE_AB, mem, c);
		break;

		/* Arithmetic */
	case ADC_IM:
		err = _ADC(self, opc, REPLACE_IM, mem, c);
		break;
	case ADC_ZP:
		err = _ADC(self, opc, REPLACE_ZP, mem, c);
		break;
	case ADC_ZX:
		err = _ADC(self, opc, REPLACE_ZX, mem, c);
		break;
	case ADC_AB:
		err = _ADC(self, opc, REPLACE_AB, mem, c);
		break;
	case ADC_AX:
		err = _ADC(self, opc, REPLACE_AX, mem, c);
		break;
	case ADC_AY:
		err = _ADC(self, opc, REPLACE_AY, mem, c);
		break;
	case ADC_IX:
		err = _ADC(self, opc, REPLACE_IX, mem, c);
		break;
	case ADC_IY:
		err = _ADC(self, opc, REPLACE_IY, mem, c);
		break;

	case SBC_IM:
		err = _SBC(self, opc, REPLACE_IM, mem, c);
		break;
	case SBC_ZP:
		err = _SBC(self, opc, REPLACE_ZP, mem, c);
		break;
	case SBC_ZX:
		err = _SBC(self, opc, REPLACE_ZX, mem, c);
		break;
	case SBC_AB:
		err = _SBC(self, opc, REPLACE_AB, mem, c);
		break;
	case SBC_AX:
		err = _SBC(self, opc, REPLACE_AX, mem, c);
		break;
	case SBC_AY:
		err = _SBC(self, opc, REPLACE_AY, mem, c);
		break;
	case SBC_IX:
		err = _SBC(self, opc, REPLACE_IX, mem, c);
		break;
	case SBC_IY:
		err = _SBC(self, opc, REPLACE_IY, mem, c);
		break;

	case CMP_IM:
		err = _CMP(self, opc, REPLACE_IM, mem, c);
		break;
	case CMP_ZP:
		err = _CMP(self, opc, REPLACE_ZP, mem, c);
		break;
	case CMP_ZX:
		err = _CMP(self, opc, REPLACE_ZX, mem, c);
		break;
	case CMP_AB:
		err = _CMP(self, opc, REPLACE_AB, mem, c);
		break;
	case CMP_AX:
		err = _CMP(self, opc, REPLACE_AX, mem, c);
		break;
	case CMP_AY:
		err = _CMP(self, opc, REPLACE_AY, mem, c);
		break;
	case CMP_IX:
		err = _CMP(self, opc, REPLACE_IX, mem, c);
		break;
	case CMP_IY:
		err = _CMP(self, opc, REPLACE_IY, mem, c);
		break;

	case CPX_IM:
		err = _CPX(self, opc, REPLACE_IM, mem, c);
		break;
	case CPX_ZP:
		err = _CPX(self, opc, REPLACE_ZP, mem, c);
		break;
	case CPX_AB:
		err = _CPX(self, opc, REPLACE_AB, mem, c);
		break;

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
