#include "nesemu/memory/video.h"

#include "nesemu/cartridge/cartridge.h"
#include "nesemu/util/error.h"
#include "nesemu/util/bits.h"

#include <stdint.h>
#include <string.h>

/**
 * Helper macro to check address bounds
 */
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
#define __CHECK_ADDRESSING(addr)                           \
	if (addr >= NESEMU_MEMORY_VRAM_ADDR_SIZE) {        \
		return NESEMU_RETURN_MEMORY_INVALILD_ADDR; \
	}
#else
#define __CHECK_ADDRESSING(addr) ;
#endif

/* -- Private Functions -- */

/**
 * Syntax sugar around cartridge reader
 */
static inline nesemu_return_t _cartridge_read(struct nes_video_memory_t *self,
					      uint16_t addr,
					      uint8_t *value)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (self->cartridge->chr_read_fn == NULL) {
		return NESEMU_RETURN_CARTRIDGE_NO_CALLBACK;
	}
#endif
	return self->cartridge->chr_read_fn(
		NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(self->cartridge), addr,
		value);
}

/**
 * Syntax sugar around cartridge writer
 */
static inline nesemu_return_t _cartridge_write(struct nes_video_memory_t *self,
					       uint16_t addr,
					       uint8_t value)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (self->cartridge->chr_write_fn == NULL) {
		return NESEMU_RETURN_CARTRIDGE_NO_CALLBACK;
	}
#endif
	return self->cartridge->chr_write_fn(
		NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(self->cartridge), addr,
		value);
}

static inline nesemu_return_t _cartridge_mirroring(
	struct nes_video_memory_t *self,
	uint16_t addr,
	uint16_t *mapped)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (self->cartridge->chr_mapper_fn == NULL) {
		return NESEMU_RETURN_CARTRIDGE_NO_CALLBACK;
	}
#endif
	return self->cartridge->chr_mapper_fn(
		NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(self->cartridge), addr,
		mapped);
}

/* -- Public Functions -- */

nesemu_return_t nes_chr_init(struct nes_video_memory_t *self,
			     struct nes_cartridge_t *cartridge)
{
	(void)memset(self, 0, sizeof(struct nes_video_memory_t));
	self->cartridge = cartridge;

	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_chr_w8(struct nes_video_memory_t *self,
			   uint16_t addr,
			   uint8_t data)
{
	__CHECK_ADDRESSING(addr);

	// Internal to PPU
	if (addr >= NESEMU_MEMORY_VRAM_PALETTE_ADDR) {
		// Compute offset from start of the array
		addr %= NESEMU_MEMORY_VRAM_PALETTE_ADDR;
		// Compute address mirroring
		addr %= NESEMU_MEMORY_VRAM_PALETTE_SIZE;

		// Set the value at the Palette RAM indexes
		self->palette_ram[addr] = data;

		// Return with no errors
		return NESEMU_RETURN_SUCCESS;
	}

	// Other addresses are mapped by the cartridge
	nesemu_return_t status = NESEMU_RETURN_SUCCESS;

	uint16_t map; // Target address
	if ((status = _cartridge_mirroring(self, addr, &map)) <
	    NESEMU_RETURN_SUCCESS) {
		return status;
	}

	/* Delegate operation to the cartridge */
	else if (status == NESEMU_INFO_CARTRIDGE_DELEGATE_RWOP) {
		return self->cartridge->chr_write_fn == NULL ?
			       // This cartridge is read-only
			       NESEMU_RETURN_CARTRIDGE_READ_ONLY :
			       // Delegate logic to cartridge
			       _cartridge_write(self, addr, data);
	}

	/* Do operation directly from PPU CIRAM */
	else if (status == NESEMU_RETURN_SUCCESS) {
		// Map address into index
		addr %= NESEMU_MEMORY_VRAM_CIRAM_SIZE;
		// Read value directly from index
		self->ciram[addr] = data;
	}

	return status;
}

nesemu_return_t nes_chr_r8(struct nes_video_memory_t *self,
			   uint16_t addr,
			   uint8_t *result)
{
	__CHECK_ADDRESSING(addr);

	// Internal to PPU
	if (addr >= NESEMU_MEMORY_VRAM_PALETTE_ADDR) {
		// Compute offset from start of the array
		addr %= NESEMU_MEMORY_VRAM_PALETTE_ADDR;
		// Compute address mirroring
		addr %= NESEMU_MEMORY_VRAM_PALETTE_SIZE;

		// Get the value at the Palette RAM indexes
		*result = self->palette_ram[addr];

		// Return with no errors
		return NESEMU_RETURN_SUCCESS;
	}

	// Other addresses are mapped by the cartridge
	nesemu_return_t status = NESEMU_RETURN_SUCCESS;

	uint16_t map; // Target address
	if ((status = _cartridge_mirroring(self, addr, &map)) <
	    NESEMU_RETURN_SUCCESS) {
		return status;
	}

	/* Delegate operation to the cartridge */
	else if (status == NESEMU_INFO_CARTRIDGE_DELEGATE_RWOP) {
		// Delegate logic to cartridge
		return _cartridge_read(self, addr, result);
	}

	/* Do operation directly from PPU CIRAM */
	else if (status == NESEMU_RETURN_SUCCESS) {
		// Map address into index
		addr %= NESEMU_MEMORY_VRAM_CIRAM_SIZE;
		// Read value directly from index
		*result = self->ciram[addr];
	}

	return status;
}

nesemu_return_t nes_chr_w16(struct nes_video_memory_t *self,
			    uint16_t addr,
			    uint16_t data)
{
	__CHECK_ADDRESSING(addr);

	// Decompose u16 into two u8
	uint8_t msb = (data & 0xFF00) >> 8, lsb = (data & 0x00FF);

	// Write LSB
	nesemu_return_t err;
	if ((err = nes_chr_w8(self, addr, lsb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Write next (MSB)
	if ((err = nes_chr_w8(self, addr + 1, msb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_chr_r16(struct nes_video_memory_t *self,
			    uint16_t addr,
			    uint16_t *result)
{
	__CHECK_ADDRESSING(addr);

	// Get the two placeholder bytes
	uint8_t lsb, msb;

	// Get LSB
	nesemu_return_t err;
	if ((err = nes_chr_r8(self, addr, &lsb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Get next (MSB)
	if ((err = nes_chr_r8(self, addr + 1, &msb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Build u16 from two u8
	*result = NESEMU_UTIL_U16(msb, lsb);

	return NESEMU_RETURN_SUCCESS;
}
