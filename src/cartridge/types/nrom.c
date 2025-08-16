#include "nesemu/cartridge/types/nrom.h"
#include "nesemu/util/compat.h"

/**
 * Helper macro to get a PRGROM address with mirroring
 *
 * $C000-$FFFF,$8000-$BFFF -> $0000-4000
 */
#define __NESEMU_CARTRIDGE_NROM_GET_ADDR(addr) \
	(addr % NESEMU_CARTRIDGE_BANK_SIZE)

nesemu_return_t nes_ines_nrom_prg_loader(nesemu_mapper_generic_ref_t self,
					uint8_t *cdata,
					size_t len)
{
	// Create a reference to `nes_ines_nrom_cartridge_t` called `this`
	NESEMU_CARTRIDGE_DEFINE_GENERIC_TYPE(
		this, struct nes_ines_nrom_cartridge_t, self);

	// Copy memory from cartridge into `this`
	(void)memcpy(this->prgrom, cdata, len);
	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_ines_nrom_prg_reader(nesemu_mapper_generic_ref_t self,
					uint16_t addr,
					uint8_t *content)
{
	// Create a reference to `nes_ines_nrom_cartridge_t` called `this`
	NESEMU_CARTRIDGE_DEFINE_GENERIC_TYPE(
		this, struct nes_ines_nrom_cartridge_t, self);

	// This cartridge has no RAM (no addresses below 0x8000 is accesible)
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (addr < NESEMU_CARTRIDGE_ROM_BEGIN) {
		return NESEMU_RETURN_CARTRIDGE_ADDR_NOT_MAPPED;
	}
#endif

	// Read contents
	*content = this->prgrom[__NESEMU_CARTRIDGE_NROM_GET_ADDR(addr)];

	return NESEMU_RETURN_SUCCESS;
}

inline nesemu_return_t nes_ines_nrom_prg_writer(nesemu_mapper_generic_ref_t self,
					       uint16_t addr,
					       uint8_t content)
{
	_NESEMU_UNUSED(self);
	_NESEMU_UNUSED(addr);
	_NESEMU_UNUSED(content);

	return NESEMU_RETURN_CARTRIDGE_READ_ONLY;
}

nesemu_return_t nes_ines_nrom_chr_loader(nesemu_mapper_generic_ref_t self,
					uint8_t *cdata,
					size_t len) 
{
}

nesemu_return_t nes_ines_nrom_chr_reader(nesemu_mapper_generic_ref_t self,
					uint16_t addr,
					uint8_t *content)
{
}
