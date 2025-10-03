#include "nesemu/cartridge/types/nrom.h"
#include "nesemu/cartridge/types/common.h"
#include <string.h>

/**
 * Helper macro to get a PRGROM address with mirroring
 *
 * $C000-$FFFF,$8000-$BFFF -> $0000-4000
 */
#define __NESEMU_CARTRIDGE_NROM_GET_ADDR(addr) \
	(addr % NESEMU_CARTRIDGE_PRGROM_BANK_SIZE)

nesemu_return_t nes_ines_nrom_prg_loader(struct nes_ines_nrom_cartridge *self,
					 uint8_t *cdata,
					 size_t len)
{
	// Copy memory from cartridge into `self`
	(void)memcpy(self->prgrom, cdata, len);
	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_ines_nrom_prg_reader(struct nes_ines_nrom_cartridge *self,
					 uint16_t addr,
					 uint8_t *content)
{
	// self cartridge has no RAM (no addresses below 0x8000 is accesible)
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (addr < NESEMU_CARTRIDGE_ROM_BEGIN) {
		return NESEMU_RETURN_CARTRIDGE_ADDR_NOT_MAPPED;
	}
#endif

	// Read contents
	*content = self->prgrom[__NESEMU_CARTRIDGE_NROM_GET_ADDR(addr)];

	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_ines_nrom_chr_loader(struct nes_ines_nrom_cartridge *self,
					 uint8_t *cdata,
					 size_t len)
{
	(void)memcpy(self->chrrom, cdata, len);
	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_ines_nrom_chr_reader(struct nes_ines_nrom_cartridge *self,
					 uint16_t addr,
					 uint8_t *content)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	/* Check within bounds */
	if (addr >= NESEMU_CARTRIDGE_NROM_CHRROM_SIZE) {
		return NESEMU_RETURN_CARTRIDGE_ADDR_NOT_MAPPED;
	}
#endif
	*content = self->chrrom[addr];
	return NESEMU_RETURN_SUCCESS;
}
