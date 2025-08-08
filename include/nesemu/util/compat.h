#ifndef __NESEMU_UTIL_COMPAT_H__
#define __NESEMU_UTIL_COMPAT_H__

/**
 * CPU instructions are built using 'switch' statements, some of the instructions
 * share the same function and thus require 'fallthrough' cases, this macro wraps
 * fallthrough attributes across compilers to avoid fallthrough warnings in this
 * explicit cases.
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202000L
#define _NESEMU_FALLTHROUGH [[fallthrough]]
#elif defined(__GNUC__) && defined(__has_c_attribute) && \
	__has_c_attribute(fallthrough)
#define _NESEMU_FALLTHROUGH \
	;                   \
	__attribute__((fallthrough))
#else
#define _NESEMU_FALLTHROUGH ;
#endif

/**
 * Alternative to an empty struct which is a GNU extension
 */
#define _NESEMU_EMPTY_TYPE \
	struct {           \
		int __foo; \
	}

#endif
