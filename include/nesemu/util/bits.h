#ifndef __NESEMU_UTIL_BITS_H__
#define __NESEMU_UTIL_BITS_H__

#include <stdint.h>

/**
 * Build a dword value using two words
 */
#define NESEMU_UTIL_U16(msb, lsb) \
    (uint16_t)(((uint16_t)msb << 8) | (uint16_t)lsb)

#endif
