/**
 * References:
 * https://www.masswerk.at/6502/6502_instruction_set.html
 */

#ifndef __NESEMU_CPU_OPS_H__
#define __NESEMU_CPU_OPS_H__

#include <stdint.h>

/**
 * Look-up table for CPU cycles for each operation
 */
extern const uint8_t nes_cpu_op_cycles[256];

/**
 * CPU addressing modes
 */
enum nes_cpu_addressing_mode {
	NESEMU_ADDRESSING_ACCUMULATOR,
	NESEMU_ADDRESSING_IMMEDIATE,
	NESEMU_ADDRESSING_ZERO_PAGE,
	NESEMU_ADDRESSING_ZERO_PAGE_X,
	NESEMU_ADDRESSING_ZERO_PAGE_Y,
	NESEMU_ADDRESSING_ABSOLUTE,
	NESEMU_ADDRESSING_ABSOLUTE_X,
	NESEMU_ADDRESSING_ABSOLUTE_Y,
	NESEMU_ADDRESSING_INDIRECT,
	NESEMU_ADDRESSING_INDIRECT_X,
	NESEMU_ADDRESSING_INDIRECT_Y,
};

/**
 * CPU interrupt vector addresses
 */
enum nes_cpu_interrupt_vector {
	NESEMU_CPU_VECTOR_NMI = 0xFFFA,
	NESEMU_CPU_VECTOR_RESET = 0xFFFC,
	NESEMU_CPU_VECTOR_IRQ = 0xFFFE,
};

/**
 * CPU instructions with their opcodes.
 *
 * Operations have suffixes that specify the addressing mode
 * _ACC = Accumulator
 * _IM = Immediate Mode
 * _ZP = Zero Page
 * _ZX = Zero Page,X
 * _AB = Absolute
 * _AX = Absolute,X
 * _AY = Absolute,Y
 * _IX = (Indirect,X)
 * _IY = (Indirect),Y
 */
enum nes_cpu_instruction_set {

	/* Load/Store Operations */
	LDA_IM = 0xA9,
	LDA_ZP = 0xA5,
	LDA_ZX = 0xB5,
	LDA_AB = 0xAD,
	LDA_AX = 0xBD,
	LDA_AY = 0xB9,
	LDA_IX = 0xA1,
	LDA_IY = 0xB1,

	LDX_IM = 0xA2,
	LDX_ZP = 0xA6,
	LDX_ZY = 0xB6,
	LDX_AB = 0xAE,
	LDX_AY = 0xBE,

	LDY_IM = 0xA0,
	LDY_ZP = 0xA4,
	LDY_ZX = 0xB4,
	LDY_AB = 0xAC,
	LDY_AX = 0xBC,

	STA_ZP = 0x85,
	STA_ZX = 0x95,
	STA_AB = 0x8D,
	STA_AX = 0x9D,
	STA_AY = 0x99,
	STA_IX = 0x81,
	STA_IY = 0x91,

	STX_ZP = 0x86,
	STX_ZY = 0x96,
	STX_AB = 0x8E,

	STY_ZP = 0x84,
	STY_ZX = 0x94,
	STY_AB = 0x8C,

	/* Register Transfers */
	TAX = 0xAA,
	TXA = 0x8A,
	TAY = 0xA8,
	TYA = 0x98,

	/* Stack operations */
	TSX = 0xBA,
	TXS = 0x9A,
	PHA = 0x48,
	PLA = 0x68,
	PHP = 0x08,
	PLP = 0x28,

	/* Logical */
	AND_IM = 0x29,
	AND_ZP = 0x25,
	AND_ZX = 0x35,
	AND_AB = 0x2D,
	AND_AX = 0x3D,
	AND_AY = 0x39,
	AND_IX = 0x21,
	AND_IY = 0x31,

	EOR_IM = 0x49,
	EOR_ZP = 0x45,
	EOR_ZX = 0x55,
	EOR_AB = 0x4D,
	EOR_AX = 0x5D,
	EOR_AY = 0x59,
	EOR_IX = 0x41,
	EOR_IY = 0x51,

	ORA_IM = 0x09,
	ORA_ZP = 0x05,
	ORA_ZX = 0x15,
	ORA_AB = 0x0D,
	ORA_AX = 0x1D,
	ORA_AY = 0x19,
	ORA_IX = 0x01,
	ORA_IY = 0x11,

	BIT_ZP = 0x24,
	BIT_AB = 0x2C,

	/* Arithmetic */
	ADC_IM = 0x69,
	ADC_ZP = 0x65,
	ADC_ZX = 0x75,
	ADC_AB = 0x6D,
	ADC_AX = 0x7D,
	ADC_AY = 0x79,
	ADC_IX = 0x61,
	ADC_IY = 0x71,

	SBC_IM = 0xE9,
	SBC_ZP = 0xE5,
	SBC_ZX = 0xF5,
	SBC_AB = 0xED,
	SBC_AX = 0xFD,
	SBC_AY = 0xF9,
	SBC_IX = 0xE1,
	SBC_IY = 0xF1,

	CMP_IM = 0xC9,
	CMP_ZP = 0xC5,
	CMP_ZX = 0xD5,
	CMP_AB = 0xCD,
	CMP_AX = 0xDD,
	CMP_AY = 0xD9,
	CMP_IX = 0xC1,
	CMP_IY = 0xD1,

	CPX_IM = 0xE0,
	CPX_ZP = 0xE4,
	CPX_AB = 0xEC,

	CPY_IM = 0xC0,
	CPY_ZP = 0xC4,
	CPY_AB = 0xCC,

	/* Increments & Decrements */
	INC_ZP = 0xE6,
	INC_ZX = 0xF6,
	INC_AB = 0xEE,
	INC_AX = 0xFE,

	INX = 0xE8,
	INY = 0xC8,

	DEC_ZP = 0xC6,
	DEC_ZX = 0xD6,
	DEC_AB = 0xCE,
	DEC_AX = 0xDE,

	DEX = 0xCA,
	DEY = 0x88,

	/* Shifts */
	ASL_ACC = 0x0A,
	ASL_ZP = 0x06,
	ASL_ZX = 0x16,
	ASL_AB = 0x0E,
	ASL_AX = 0x1E,

	LSR_ACC = 0x4A,
	LSR_ZP = 0x46,
	LSR_ZX = 0x56,
	LSR_AB = 0x4E,
	LSR_AX = 0x5E,

	ROL_ACC = 0x2A,
	ROL_ZP = 0x26,
	ROL_ZX = 0x36,
	ROL_AB = 0x2E,
	ROL_AX = 0x3E,

	ROR_ACC = 0x6A,
	ROR_ZP = 0x66,
	ROR_ZX = 0x76,
	ROR_AB = 0x6E,
	ROR_AX = 0x7E,

	/* Jumps & Calls */
	JMP_AB = 0x4C,
	JMP_IX = 0x6C,

	JSR = 0x20,
	RTS = 0x60,

	/* Branches */
	BCC = 0x90,
	BCS = 0xB0,
	BEQ = 0xF0,
	BMI = 0x30,
	BNE = 0xD0,
	BPL = 0x10,
	BVC = 0x50,
	BVS = 0x70,

	/* Status Flag Changes */
	CLC = 0x18,
	CLD = 0xD8,
	CLI = 0x58,
	CLV = 0xB8,
	SEC = 0x38,
	SED = 0xF8,
	SEI = 0x78,

	/* System Functions */
	BRK = 0x00,
	NOP = 0xEA,
	RTI = 0x40,

    /* Unofficial Opcodes */
    STP = 0xDB,
};

#endif
