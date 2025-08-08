#ifndef __NESEMU_UTIL_ERROR_H__
#define __NESEMU_UTIL_ERROR_H__

/**
 * Nesemu library error codes
 */
typedef enum nesemu_error_t {

	/* Generic Errors */
	NESEMU_RETURN_SUCCESS = 0, /**< Operation success */
	NESEMU_RETURN_GENERIC_ERROR = -1, /**< Unknown error (check errno) */
	NESEMU_RETURN_BAD_ARGUMENTS = -2,

	/* Memory */
	NESEMU_RETURN_MEMORY_INVALILD_ADDR = -0x10,
	NESEMU_RETURN_MEMORY_STACK_OVERFLOW = -0x11,
	NESEMU_RETURN_MEMORY_STACK_UNDERFLOW = -0x12,
	NESEMU_RETURN_MEMORY_PRGROM_OVERFLOW = -0x13,
	NESEMU_RETURN_MEMORY_PRGROM_NO_DATA = -0x14,

	/* CPU */
	NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION = -0x20,
	NESEMU_RETURN_CPU_BAD_ADDRESSING = -0x21,

	/* Cartridge */
	NESEMU_RETURN_CARTRIDGE_BAD_INES_FORMAT = -0x30,
	NESEMU_RETURN_CARTRIDGE_TOO_LARGE = -0x31,
	NESEMU_RETURN_CARTRIDGE_EMPTY = -0x32,
	NESEMU_RETURN_CARTRIDGE_UNSUPPORTED_MAPPER = -0x33,
	NESEMU_RETURN_CARTRIDGE_UNMAPPED_ADDR = -0x34,
	NESEMU_RETURN_CARTRIDGE_READ_ONLY = -0x35,

} nesemu_error_t;

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
