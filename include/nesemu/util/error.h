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

	/* Memory */
	NESEMU_RETURN_MEMORY_INVALILD_ADDR,
    NESEMU_RETURN_MEMORY_STACK_OVERFLOW,
    NESEMU_RETURN_MEMORY_STACK_UNDERFLOW,

} nesemu_error_t;

#endif
