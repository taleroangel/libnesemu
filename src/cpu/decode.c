/**
 * This file contains definitions for functions in 'cpu.h'
 * Contains code required to decode CPU instructions
 */

#include "nesemu/cpu/cpu.h"
#include "nesemu/cpu/instructions.h"
#include "nesemu/cpu/status.h"

#include "nesemu/memory/memory.h"
#include "nesemu/memory/paging.h"

#include "nesemu/util/error.h"
#include "nesemu/util/bits.h"

#include <stddef.h>
#include <stdbool.h>

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
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		break;

	case LDA_AB:
		// Load Accumulator Absolute
		// Load contents at memory address
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		err = nes_mem_r8(mem, NESEMU_UTIL_U16(msb, lsb), &self->a);
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
		// Contents in ptr address will be the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		// Read contents from the actual address into acc register
		err = nes_mem_r8(mem, addr, &self->a);
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->a));
		break;

	case LDA_IY:
		// Load Accumulator, Indirect Y (Post-Indexed)
		// Load the pointer address (always zero page)
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		// Contents in ptr address will be the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		// Add Y to the actual address
		addr += self->y;
		// Read contents from the actual address into acc register
		err = nes_mem_r8(mem, addr, &self->a);
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
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->x));
		break;

	case LDX_AB:
		// Load X Absolute
		// Load contents at memory address
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		err = nes_mem_r8(mem, NESEMU_UTIL_U16(msb, lsb), &self->x);
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
		// Update status flags
		nes_cpu_status_mask_set(self,
					NESEMU_CPU_STATUS_MASK_NZ(self->y));
		break;

	case LDY_AB:
		// Load Y Absolute
		// Load contents at memory address
		lsb = nes_cpu_fetch(self, mem), msb = nes_cpu_fetch(self, mem);
		err = nes_mem_r8(mem, NESEMU_UTIL_U16(msb, lsb), &self->y);
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
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
		// Write $acc into actual address
		err = nes_mem_w8(mem, addr, self->a);
		break;

	case STA_IY:
		// Store Accumulator, Indirect Y (Post-Indexed)
		// Load the pointer address (always zero page)
		ptr = NESEMU_ZEROPAGE_GET_ADDR(nes_cpu_fetch(self, mem));
		// Contents in ptr address will be the actual address
		err = nes_mem_r16(mem, ptr, &addr);
		if (err != NESEMU_RETURN_SUCCESS) {
			return err;
		}
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

		/* Stack operations */
	case TSX:
	case TXS:
	case PHA:
	case PLA:
	case PHP:
	case PLP:

		/* Logical */
	case AND_IM:
	case AND_ZP:
	case AND_ZX:
	case AND_AB:
	case AND_AX:
	case AND_AY:
	case AND_IX:
	case AND_IY:

	case EOR_IM:
	case EOR_ZP:
	case EOR_ZX:
	case EOR_AB:
	case EOR_AX:
	case EOR_AY:
	case EOR_IX:
	case EOR_IY:

	case ORA_IM:
	case ORA_ZP:
	case ORA_ZX:
	case ORA_AB:
	case ORA_AX:
	case ORA_AY:
	case ORA_IX:
	case ORA_IY:

	case BIT_ZP:
	case BIT_AB:

		/* Arithmetic */
	case ADC_IM:
	case ADC_ZP:
	case ADC_ZX:
	case ADC_AB:
	case ADC_AX:
	case ADC_AY:
	case ADC_IX:
	case ADC_IY:

	case SBC_IM:
	case SBC_ZP:
	case SBC_ZX:
	case SBC_AB:
	case SBC_AX:
	case SBC_AY:
	case SBC_IX:
	case SBC_IY:

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
