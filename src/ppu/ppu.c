#include "nesemu/ppu/ppu.h"
#include "nesemu/memory/main.h"
#include "nesemu/ppu/palette.h"
#include "nesemu/util/error.h"

#include <stdlib.h>

/**
 * Framebuffer for video output
 *
 * @note Only for type punning.
 */
union nes_ppu_framebuffer {

	/** Framebuffer (RGB24 Format) as raw bytes */
	nes_display_t bytes;

	/** Framebuffer in RGB24 components representation */
	struct nes_ppu_rgb24 {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} __attribute__((packed))
	rgb24[NESEMU_PPU_SCREEN_HEIGHT * NESEMU_PPU_SCREEN_WIDTH];
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
nesemu_return_t nes_ppu_init(struct nes_ppu *self,
			     nes_ppu_palette_t *system_palette,
			     struct nes_mem_main *mem)
{
	nesemu_return_t err = NESEMU_RETURN_SUCCESS;

#ifndef NESEMU_DISABLE_SAFETY_CHECKS
    if (mem == NULL) {
        return NESEMU_RETURN_BAD_ARGUMENTS;
    }
    if (system_palette == NULL) {
        return NESEMU_RETURN_PPU_BAD_PALETTE;
    }
#endif

    // Set system palette
    self->system_palette = system_palette;

	// Start with the pre-render scanline (scanline -1)
	self->scanline = NESEMU_PPU_NTSC_PRERENDER_SCANLINE;
    self->dot = 0;

    // Set PPUSTATUS initial status
    // Other flags are 0 so no additional initialization required
    err = nes_mem_w8(mem, NESEMU_PPU_REG_PPUSTATUS, 0xA0);
    _NESEMU_RETURN_IF_ERR(err);

	return err;
}

nesemu_return_t nes_ppu_render(struct nes_ppu *self,
			       nes_display_t *display,
			       struct nes_mem_main *mem,
			       struct nes_mem_video *vim,
			       int *cycles)
{
    // Set number of cycles operation took
    *cycles = 1;

    /* Visible scanlines */
    if (self->scanline <= NESEMU_PPU_NTSC_RENDERING_SCANLINES) {
        /* Skip, Idle */
        if (self->dot == 0) {
        }
        else if (self->dot <= 256) {
        }
        else if (self->dot <= 336) {
        }
        else {
        }
    }
    /* Idle section */
    else if (self->scanline == NESEMU_PPU_NTSC_IDLE_SCANLINE) {
        // Fast-forward to next scanline
        self->dot = 0;
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
    self->dot = (self->dot + 1) % NESEMU_PPU_NTSC_DOTS_PER_SCANLINE;
    // Next scanline on rasterline completion, else keep same scanline
    self->scanline = (self->dot == 0) ?
			     (self->scanline + 1) % NESEMU_PPU_NTSC_SCANLINES :
			     self->scanline;
    // On frame completion, switch to 1 or 0 
    self->frame = (self->scanline == 0) && (self->dot == 0) ?
			self->frame ^ 1 :
			self->frame;

    return EXIT_SUCCESS;
}
