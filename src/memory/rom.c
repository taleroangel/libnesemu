#include "nesemu/memory/rom.h"
#include "nesemu/util/error.h"

#include <stddef.h>
#include <string.h>

nesemu_error_t nes_memory_wprgmem(nes_memory_t mem, uint8_t *data, size_t len)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
    if (data == NULL) {
        return NESEMU_RETURN_BAD_ARGUMENTS;
    }
    if (len > NESEMU_MEMORY_PRGMEM_SIZE) {
        return NESEMU_RETURN_MEMORY_PRGMEM_OVERFLOW;
    }
#endif
    (void)memcpy(mem + NESEMU_MEMORY_PRGMEM_BEGIN, data, len);
	return NESEMU_RETURN_SUCCESS;
}
