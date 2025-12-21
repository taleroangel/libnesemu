#include "nesemu/cartridge/types/nrom.h"
#include "nesemu/cartridge/types/common.h"
#include <string.h>

nesemu_return_t nes_ines_nrom_prg_loader(struct nes_ines_nrom_cartridge *self,
					 uint8_t *cdata,
					 size_t len)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (len > NESEMU_CARTRIDGE_NROM_PRGROM_SIZE) {
		return NESEMU_RETURN_CARTRIDGE_TOO_LARGE;
	}
#endif

	// Check if address should be mirrored
	self->mirrored_bank = (len <= (NESEMU_CARTRIDGE_NROM_PRGROM_SIZE / 2));

	// Copy memory from cartridge into `self`
	(void)memcpy(&(self->prgrom), cdata, len);
	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_ines_nrom_prg_reader(struct nes_ines_nrom_cartridge *self,
					 uint16_t addr,
					 uint8_t *content)
{
	// self cartridge has no RAM (no addresses belowcesible)
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (addr < NESEMU_CARTRIDGE_ROM_BEGIN) {
		return NESEMU_RETURN_CARTRIDGE_ADDR_NOT_MAPPED;
	}
#endif

	// Read contents
	*content =
        // Address should be mirrored if (mirrored_bank) is true
        // Also, remove the offset since internal address starts at $0000
		self->prgrom[self->mirrored_bank ?
				     (addr % NESEMU_CARTRIDGE_PRGROM_BANK_SIZE) :
				     (addr % NESEMU_CARTRIDGE_NROM_PRGROM_SIZE)];
	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_ines_nrom_chr_loader(struct nes_ines_nrom_cartridge *self,
					 uint8_t *cdata,
					 size_t len)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (len > NESEMU_CARTRIDGE_NROM_CHRROM_SIZE) {
		return NESEMU_RETURN_CARTRIDGE_TOO_LARGE;
	}
#endif
	(void)memcpy(&(self->chrrom), cdata, len);
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
