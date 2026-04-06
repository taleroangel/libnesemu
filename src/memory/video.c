#include "nesemu/memory/video.h"

#include "nesemu/cartridge/cartridge.h"
#include "nesemu/cartridge/types/common.h"
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
static inline nesemu_return_t _cartridge_read(struct nes_mem_video *self,
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
static inline nesemu_return_t _cartridge_write(struct nes_mem_video *self,
					       uint16_t addr,
					       uint8_t value)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	if (self->cartridge->chr_write_fn == NULL) {
		return NESEMU_RETURN_CARTRIDGE_CHRROM_READ_ONLY;
	}
#endif
	return self->cartridge->chr_write_fn(
		NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(self->cartridge), addr,
		value);
}

/**
 * Syntax sugar around cartridge mapper
 */
static inline nesemu_return_t _cartridge_mapping(struct nes_mem_video *self,
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

nesemu_return_t nes_vram_init(struct nes_mem_video *self,
			      struct nes_cartridge *cartridge)
{
	(void)memset(self, 0, sizeof(struct nes_mem_video));
	self->cartridge = cartridge;

	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_vram_w8(struct nes_mem_video *self,
			    uint16_t addr,
			    uint8_t data)
{
	__CHECK_ADDRESSING(addr);

	// Internal to PPU
	if (addr >= NESEMU_MEMORY_VRAM_PALETTE_ADDR) {
		// Compute offset from start of the array
		addr %= NESEMU_MEMORY_VRAM_PALETTE_ADDR;
		// Compute address mirroring
		addr %= NESEMU_MEMORY_VRAM_PALETTE_RAM_SIZE;

		// Set the value at the Palette RAM indexes
		self->palette_ram[addr] = data;

		// Return with no errors
		return NESEMU_RETURN_SUCCESS;
	}

	// Other addresses are mapped by the cartridge
	nesemu_return_t status = NESEMU_RETURN_SUCCESS;

    // Address mapping by cartridge into same variable
	if ((status = _cartridge_mapping(self, addr, &addr)) < NESEMU_RETURN_SUCCESS) {
		// Handle error
		return status;
	}

	/* Delegate operation to the cartridge */
	else if (status == NESEMU_INFO_CARTRIDGE_DELEGATE_RWOP) {
		return self->cartridge->chr_write_fn == NULL ?
			       // This cartridge is read-only
			       NESEMU_RETURN_CARTRIDGE_CHRROM_READ_ONLY :
			       // Delegate logic to cartridge
			       _cartridge_write(self, addr, data);
	}

	/* Do operation directly from PPU CIRAM */
	else if (status == NESEMU_RETURN_SUCCESS) {
		// Check that address is within range
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
		if (addr < NESEMU_MEMORY_VRAM_CIRAM_ADDR) {
			return NESEMU_RETURN_MEMORY_VRAM_BAD_MAPPER;
		}
#endif
		// Map address into index
		addr %= NESEMU_MEMORY_VRAM_CIRAM_SIZE;
		// Read value directly from index
		self->ciram[addr] = data;
	}

	return status;
}

nesemu_return_t nes_vram_r8(struct nes_mem_video *self,
			    uint16_t addr,
			    uint8_t *result)
{
	__CHECK_ADDRESSING(addr);

	// Internal to PPU
	if (addr >= NESEMU_MEMORY_VRAM_PALETTE_ADDR) {
		// Compute offset from start of the array
		addr %= NESEMU_MEMORY_VRAM_PALETTE_ADDR;
		// Compute address mirroring
		addr %= NESEMU_MEMORY_VRAM_PALETTE_RAM_SIZE;

		// Get the value at the Palette RAM indexes
		*result = self->palette_ram[addr];

		// Return with no errors
		return NESEMU_RETURN_SUCCESS;
	}

	// Other addresses are mapped by the cartridge
	nesemu_return_t status = NESEMU_RETURN_SUCCESS;

    // Address mapping by cartridge into the same var
	if ((status = _cartridge_mapping(self, addr, &addr)) < NESEMU_RETURN_SUCCESS) {
		// Handle error
		return status;
	}

	/* Delegate operation to the cartridge */
	else if (status == NESEMU_INFO_CARTRIDGE_DELEGATE_RWOP) {
		// Delegate logic to cartridge
		return _cartridge_read(self, addr, result);
	}

	/* Do operation directly from PPU CIRAM */
	else if (status == NESEMU_RETURN_SUCCESS) {
		// Check that address is within range
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
		if (addr < NESEMU_MEMORY_VRAM_CIRAM_ADDR) {
			return NESEMU_RETURN_MEMORY_VRAM_BAD_MAPPER;
		}
#endif
		// Map address into index
		addr %= NESEMU_MEMORY_VRAM_CIRAM_SIZE;
		// Read value directly from index
		*result = self->ciram[addr];
	}

	return status;
}

nesemu_return_t nes_vram_w16(struct nes_mem_video *self,
			     uint16_t addr,
			     uint16_t data)
{
	__CHECK_ADDRESSING(addr);

	// Decompose u16 into two u8
	uint8_t msb = (data & 0xFF00) >> 8, lsb = (data & 0x00FF);

	// Write LSB
	nesemu_return_t err;
	if ((err = nes_vram_w8(self, addr, lsb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Write next (MSB)
	if ((err = nes_vram_w8(self, addr + 1, msb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_vram_r16(struct nes_mem_video *self,
			     uint16_t addr,
			     uint16_t *result)
{
	__CHECK_ADDRESSING(addr);

	// Get the two placeholder bytes
	uint8_t lsb, msb;

	// Get LSB
	nesemu_return_t err;
	if ((err = nes_vram_r8(self, addr, &lsb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Get next (MSB)
	if ((err = nes_vram_r8(self, addr + 1, &msb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Build u16 from two u8
	*result = NESEMU_UTIL_U16(msb, lsb);

	return NESEMU_RETURN_SUCCESS;
}

nesemu_return_t nes_vram_pattern_read(struct nes_mem_video *self,
				      uint16_t addr,
				      nes_vram_pattern_t *pattern)
{
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
	// Check if address if greater than pattern table addressable space
	if (addr >= NESEMU_CARTRIDGE_PATTERN_TABLE_ADDR) {
		return NESEMU_RETURN_MEMORY_INVALILD_ADDR;
	}
#endif

	// Get address map by the cartridge
	nesemu_return_t status = NESEMU_RETURN_SUCCESS;

    // Address mapping by cartridge into the same address
	if ((status = _cartridge_mapping(self, addr, &addr)) < NESEMU_RETURN_SUCCESS) {
		// Handle error
		return status;
	}

	/* Delegate operation to the cartridge */
	else if (status == NESEMU_INFO_CARTRIDGE_DELEGATE_RWOP) {
		// Check if cartridge has read function
#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
		if (self->cartridge->chr_read_fn == NULL) {
			return NESEMU_RETURN_CARTRIDGE_NO_CALLBACK;
		}
#endif
        // For each byte, delegate operation to cartridge
        for (size_t idx = 0; idx < NESEMU_MEMORY_VRAM_PATTERN_SIZE; idx++) {
            if ((status = self->cartridge->chr_read_fn(
                            NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF(self->cartridge),
                            addr + idx,
                            pattern[idx])
                ) < NESEMU_RETURN_SUCCESS)
            {
                return status;
            }
        }
	}

	/* Pattern tables only live in cartridge */
	else if (status == NESEMU_RETURN_SUCCESS) {
        return NESEMU_RETURN_MEMORY_VRAM_BAD_MAPPER;
	}

	return status;
}

nesemu_return_t nes_vram_palette_read(struct nes_mem_video *self,
				      uint16_t addr,
				      nes_vram_palette_t *palette)
{

#ifndef CONFIG_NESEMU_DISABLE_SAFETY_CHECKS
    __CHECK_ADDRESSING(addr);
    if (addr < NESEMU_MEMORY_VRAM_PALETTE_ADDR) {
        return NESEMU_RETURN_MEMORY_INVALILD_ADDR;
    }
#endif
    // Compute offset from start of the array
    addr %= NESEMU_MEMORY_VRAM_PALETTE_ADDR;
    // Compute address mirroring
    addr %= NESEMU_MEMORY_VRAM_PALETTE_RAM_SIZE;

    // Copy contents to target variable
    for (size_t idx = 0; idx < NESEMU_MEMORY_VRAM_PALETTE_SIZE; idx++) {
        *palette[idx] = self->palette_ram[addr + idx];
    }

    // Return with no errors
    return NESEMU_RETURN_SUCCESS;

}
