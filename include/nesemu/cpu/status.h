/**
 * References:
 * https://www.nesdev.org/wiki/CPU_ALL#Status_flag_behavior
 */

#ifndef __NESEMU_CPU_STATUS_H__
#define __NESEMU_CPU_STATUS_H__

/**
 * Bitmasks for the CPU status flags
 */
enum cpu_status_t {
	NESEMU_CPU_FLAGS_C = 0b00000001, /**< Carry */
	NESEMU_CPU_FLAGS_Z = 0b00000010, /**< Zero */
	NESEMU_CPU_FLAGS_I = 0b00000100, /**< Interrupt disable */
	NESEMU_CPU_FLAGS_D = 0b00001000, /**< Decimal mode (No effect) */
	NESEMU_CPU_FLAGS_B = 0b00010000, /**< Break command */
	NESEMU_CPU_FLAGS_1 = 0b00100000, /**< Unused! */
	NESEMU_CPU_FLAGS_V = 0b01000000, /**< Overflow */
	NESEMU_CPU_FLAGS_N = 0b10000000, /**< Negative */
};

/**
 * Returns the appropiate CPU status mask for the N flag given a value
 */
#define NESEMU_CPU_STATUS_MASK_N(value) \
	(uint8_t)((uint8_t)value & (uint8_t)NESEMU_CPU_FLAGS_N)

/**
 * Returns the appropiate CPU status mask for the Z flag given a value
 */
#define NESEMU_CPU_STATUS_MASK_Z(value) \
    (uint8_t)((value == 0) ? NESEMU_CPU_FLAGS_Z : 0)

/**
 * Returns the appropiate CPU status mask for both the N & Z flags given a value
 */
#define NESEMU_CPU_STATUS_MASK_NZ(value) \
    (NESEMU_CPU_STATUS_MASK_N(value) | NESEMU_CPU_STATUS_MASK_Z(value))

/**
 * Returns the result of setting flags from the mask into status
 */
#define NESEMU_CPU_STATUS_SET_MASK(status, mask) \
    (uint8_t)status | (uint8_t)mask

/**
 * Returns the result of removing flags from the mask from status
 */
#define NESEMU_CPU_STATUS_UNSET_MASK(status, mask) \
    (uint8_t)status ^ (uint8_t)mask

#endif
