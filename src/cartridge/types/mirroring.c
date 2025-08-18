#include "nesemu/cartridge/types/mirroring.h"
#include "nesemu/util/compat.h"
#include "nesemu/util/error.h"

nesemu_return_t nes_cartridge_mapper_horizontal(nesemu_mapper_generic_ref_t self,
						uint16_t addr,
						uint16_t *mapped)
{
    _NESEMU_UNUSED(self);

    /* CHRROM. Delegate operation to cartridge */
    if (addr < NESEMU_CARTRIDGE_PATTERN_TABLE_ADDR) {
        *mapped = addr;
        return NESEMU_INFO_CARTRIDGE_DELEGATE_RWOP;
    }

    /* A, A, B, B */
	*mapped = (addr < 0x2800) ?
        ((addr % 0x400) + 0x2000) /* A */ :
		((addr % 0x400) + 0x2400) /* B */;

	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_cartridge_mapper_vertical(nesemu_mapper_generic_ref_t self,
					      uint16_t addr,
					      uint16_t *mapped)
{
    _NESEMU_UNUSED(self);

    /* CHRROM. Delegate operation to cartridge */
    if (addr < NESEMU_CARTRIDGE_PATTERN_TABLE_ADDR) {
        *mapped = addr;
        return NESEMU_INFO_CARTRIDGE_DELEGATE_RWOP;
    }

    /* A, B, A, B */
	*mapped = (addr < 0x2800) ?
        (addr) /* AB */ :
		((addr % 0x2800) + 0x2000) /* AB (mirroring) */;

	return NESEMU_RETURN_SUCCESS;
}
