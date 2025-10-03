#include "nesemu/ppu/ppu.h"
#include "nesemu/memory/main.h"
#include "nesemu/util/error.h"

#include <string.h>
#include <stdlib.h>

/**
 * Framebuffer for video output
 *
 * @note Only for type punning.
 */
union nes_ppu_framebuffer_t {

	/** Framebuffer (RGB24 Format) as raw bytes */
	nes_display_t bytes;

	/** Framebuffer in RGB24 components representation */
	struct rgb24_t {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} rgb24[NESEMU_PPU_SCREEN_HEIGHT * NESEMU_PPU_SCREEN_WIDTH];
};

/* --- Constants --- */

/** Number of PPU dots/cycles per scanline */
#define NESEMU_PPU_NTSC_DOTS_PER_SCANLINE 341

/** Number of scanlines in NTSC format */
#define NESEMU_PPU_NTSC_SCANLINES 262

/** Last index for the last rendering scanline */
#define NESEMU_PPU_NTSC_RENDERING_SCANLINES 239

/** Index of the post-render scanline */
#define NESEMU_PPU_NTSC_IDLE_SCANLINE 240

/** Last index for the vertical blank section */
#define NESEMU_PPU_NTSC_VBLANK_SCANLINE 260

/** Index for the pre-render scanline */
#define NESEMU_PPU_NTSC_PRERENDER_SCANLINE 261

/* --- Function Definition --- */
nesemu_return_t nes_ppu_init(struct nes_ppu_t *self, struct nes_main_memory_t *mem)
{
    nesemu_return_t err = NESEMU_RETURN_SUCCESS;

    // Start with the pre-render scanline (scanline -1)
	self->scanline = NESEMU_PPU_NTSC_PRERENDER_SCANLINE;
    self->rasterline = 0;

    // Set PPUSTATUS initial status
    // Other flags are 0 so no additional initialization required
    err = nes_mem_w8(mem, NESEMU_PPU_REG_PPUSTATUS, 0xA0);
    _NESEMU_RETURN_IF_ERR(err);

	return err;
}

nesemu_return_t nes_ppu_render(struct nes_ppu_t *self,
			       nes_display_t *display,
			       struct nes_main_memory_t *mem,
			       struct nes_video_memory_t *vim,
			       int *cycles)
{
    // Set number of cycles operation took
    *cycles = 1;

    /* Visible scanlines */
    if (self->scanline <= NESEMU_PPU_NTSC_RENDERING_SCANLINES) {
        if (self->rasterline == 0) {
        }
        else if (self->rasterline <= 256) {
        }
        else if (self->rasterline <= 336) {
        }
        else {
        }
    }
    /* Idle section */
    else if (self->scanline == NESEMU_PPU_NTSC_IDLE_SCANLINE) {
        // Fast-forward to next scanline
        self->rasterline = 0;
        self->scanline += 1;
        *cycles = NESEMU_PPU_NTSC_DOTS_PER_SCANLINE;

        return EXIT_SUCCESS;
    }
    /* VBlank */
    else if (self->scanline <= NESEMU_PPU_NTSC_VBLANK_SCANLINE) {
    }
    /* Pre-render */
    else if (self->scanline == NESEMU_PPU_NTSC_PRERENDER_SCANLINE) {
    }

    // Next rasterline, rollback to 0 on scanline end
    self->rasterline = (self->rasterline + 1) % NESEMU_PPU_NTSC_DOTS_PER_SCANLINE;
    // Next scanline on rasterline completion, else keep same scanline
    self->scanline = (self->rasterline == 0) ?
			     (self->scanline + 1) % NESEMU_PPU_NTSC_SCANLINES :
			     self->scanline;
    // On frame completion, switch to 1 or 0 
    self->odd = (self->scanline == 0) && (self->rasterline == 0) ?
			self->odd ^ 1 :
			self->odd;

    return EXIT_SUCCESS;
}
