#ifndef __NESEMU_UTIL_ERROR_H__
#define __NESEMU_UTIL_ERROR_H__

/**
 * Nesemu library return error/information codes.
 *
 * Code <0  -> Error,
 * Code =0  -> Success,
 * Code >0  -> Information
 */
typedef enum nesemu_return {

	/* --- Generic Errors --- */
	NESEMU_RETURN_SUCCESS = 0, /**< Operation success */
	NESEMU_RETURN_GENERIC_ERROR = -1, /**< Unknown error (check errno if available) */
	NESEMU_RETURN_BAD_ARGUMENTS = -2, /**< Wrong arguments for function call */

	/* --- Memory --- */
	NESEMU_RETURN_MEMORY_INVALILD_ADDR = -0x10,
	NESEMU_RETURN_MEMORY_STACK_OVERFLOW = -0x11,
	NESEMU_RETURN_MEMORY_STACK_UNDERFLOW = -0x12,
	NESEMU_RETURN_MEMORY_PRGROM_OVERFLOW = -0x13,
	NESEMU_RETURN_MEMORY_PRGROM_NO_DATA = -0x14,
	NESEMU_RETURN_MEMORY_VRAM_BAD_MAPPER = -0x15,

	/* --- CPU --- */
	NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION = -0x20,
	NESEMU_RETURN_CPU_BAD_ADDRESSING = -0x21,

	/* --- Cartridge --- */

	/* Errors */
	NESEMU_RETURN_CARTRIDGE_BAD_INES_FORMAT = -0x30,
	NESEMU_RETURN_CARTRIDGE_TOO_LARGE = -0x31,
	NESEMU_RETURN_CARTRIDGE_EMPTY = -0x32,
	NESEMU_RETURN_CARTRIDGE_UNSUPPORTED_MAPPER = -0x33,
	NESEMU_RETURN_CARTRIDGE_ADDR_NOT_MAPPED = -0x34,
	NESEMU_RETURN_CARTRIDGE_NO_CALLBACK = -0x35,
	NESEMU_RETURN_CARTRIDGE_CHRROM_READ_ONLY = -0x36,
	NESEMU_RETURN_CARTRIDGE_PRGROM_READ_ONLY = -0x37,

	/* Information */

	/**
     * Read/Write operation should be delegated to the cartridge's r/w callbacks
     * instead of being handled by the bus directly.
     */
	NESEMU_INFO_CARTRIDGE_DELEGATE_RWOP = 0x30,

} nesemu_return_t;

/**
 * Helper macro to return from a function with (nesemu_error_t) return type
 * Reduces boilerplate and can be disabled with -DCONFIG_NESEMU_DISABLE_SAFETY_CHECKS
 */
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
#define _NESEMU_RETURN_IF_ERR(err)            \
	if ((err) != NESEMU_RETURN_SUCCESS) { \
		return err;                   \
	}
#else
#define _NESEMU_RETURN_IF_ERR(err) ;
#endif

#endif
