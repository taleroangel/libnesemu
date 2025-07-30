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

    /* CPU */
    NESEMU_RETURN_CPU_UNSUPPORTED_INSTRUCTION,
    NESEMU_RETURN_CPU_BAD_ADDRESSING,

	/* Memory */
	NESEMU_RETURN_MEMORY_INVALILD_ADDR,
    NESEMU_RETURN_MEMORY_STACK_OVERFLOW,
    NESEMU_RETURN_MEMORY_STACK_UNDERFLOW,
    NESEMU_RETURN_MEMORY_PRGMEM_OVERFLOW,

} nesemu_error_t;

/**
 * Helper macro to return from a function with (nesemu_error_t) return type
 * Reduces boilerplate and can be disabled with -DCONFIG_NESEMU_DISABLE_SAFETY_CHECKS
 */
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
#define _NESEMU_RETURN_IF_ERR(err)  \
	if ((err) != NESEMU_RETURN_SUCCESS) { return err; }
#else
#define _NESEMU_RETURN_IF_ERR(err) ;
#endif

#endif
