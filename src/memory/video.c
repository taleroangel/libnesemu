#include "nesemu/memory/video.h"

#include "nesemu/util/error.h"
#include "nesemu/util/bits.h"

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

nesemu_error_t nes_chr_reset(struct nes_video_memory_t *mem)
{
	(void)memset(mem, 0, sizeof(struct nes_video_memory_t));
	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_chr_w8(struct nes_video_memory_t *mem,
			  uint16_t addr,
			  uint8_t data)
{
	__CHECK_ADDRESSING(addr);

	// Delegate to cartridge ppu callback
	if (addr <= NESEMU_MEMORY_VRAM_CARTRIDGE_END) {
		// Must return!, The callback must handle the logic
		return nes_chr_cartridge_write(mem, addr, data);
	}

	// Write at address with mirroring considered
	mem->_data[addr % NESEMU_MEMORY_VRAM_BASE] = data;

	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_chr_r8(struct nes_video_memory_t *mem,
			  uint16_t addr,
			  uint8_t *result)
{
	__CHECK_ADDRESSING(addr);

	// Delegate to cartridge ppu callback
	if (addr <= NESEMU_MEMORY_VRAM_CARTRIDGE_END) {
		// Must return!, The callback must handle the logic
		return nes_chr_cartridge_read(mem, addr, result);
	}

	// Read at address with mirroring considered
	*result = mem->_data[addr % NESEMU_MEMORY_VRAM_BASE];

	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_chr_w16(struct nes_video_memory_t *mem,
			   uint16_t addr,
			   uint16_t data)
{
	__CHECK_ADDRESSING(addr);

	// Decompose u16 into two u8
	uint8_t msb = (data & 0xFF00) >> 8, lsb = (data & 0x00FF);

	// Write LSB
	nesemu_error_t err;
	if ((err = nes_chr_w8(mem, addr, lsb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Write next (MSB)
	if ((err = nes_chr_w8(mem, addr + 1, msb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	return NESEMU_RETURN_SUCCESS;
}

nesemu_error_t nes_chr_r16(struct nes_video_memory_t *mem,
			   uint16_t addr,
			   uint16_t *result)
{
	__CHECK_ADDRESSING(addr);

	// Get the two placeholder bytes
	uint8_t lsb, msb;

	// Get LSB
	nesemu_error_t err;
	if ((err = nes_chr_r8(mem, addr, &lsb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Get next (MSB)
	if ((err = nes_chr_r8(mem, addr + 1, &msb)) != NESEMU_RETURN_SUCCESS) {
		return err;
	}

	// Build u16 from two u8
	*result = NESEMU_UTIL_U16(msb, lsb);

	return NESEMU_RETURN_SUCCESS;
}
