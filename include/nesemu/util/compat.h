#ifndef __NESEMU_UTIL_COMPAT_H__
#define __NESEMU_UTIL_COMPAT_H__

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 202000L
#define _NES_NESEMU_FALLTHROUGH [[fallthrough]]
#elif defined (__GNUC__) && defined (__has_c_attribute) && __has_c_attribute(fallthrough)
#define _NESEMU_FALLTHROUGH ;__attribute__((fallthrough))
#else
#define _NESEMU_FALLTHROUGH ;
#endif

#endif
