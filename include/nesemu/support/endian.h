#ifndef __NESEMU_SUPPORT_ENDIAN_H__
#define __NESEMU_SUPPORT_ENDIAN_H__

#if defined (NESEMU_SUPPORT_LITTLE_ENDIAN) && defined (NESEMU_SUPPORT_BIG_ENDIAN)
#error "Cannot define both 'NESEMU_SUPPORT_LITTLE_ENDIAN' and 'NESEMU_SUPPORT_BIG_ENDIAN', select your target platform endianness"
#endif

/* If neither NESEMU_SUPPORT_LITTLE_ENDIAN or NESEMU_SUPPORT_BIG_ENDIAN are defined */
#if !defined(NESEMU_SUPPORT_LITTLE_ENDIAN) && !defined(NESEMU_SUPPORT_BIG_ENDIAN)
/* Infer endianness from compiler definitions */
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define NESEMU_SUPPORT_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define NESEMU_SUPPORT_BIG_ENDIAN 1
/* Infer endianness from the architecture */
#elif defined(__amd64__) || defined(__x86_64__) || defined(__arm__) || defined (__aarch64__) || defined (__i386__)
#define NESEMU_SUPPORT_LITTLE_ENDIAN 1
#else
#error "Could not infer endianness, please definee either 'NESEMU_SUPPORT_LITTLE_ENDIAN' or 'NESEMU_SUPPORT_BIG_ENDIAN'"
#endif
#endif

#endif
