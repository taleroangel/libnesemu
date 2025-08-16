#ifndef __NESEMU_CARTRIDGE_TYPES_COMMON_H__
#define __NESEMU_CARTRIDGE_TYPES_COMMON_H__

#include "nesemu/util/error.h"

#include <stdint.h>
#include <stddef.h>

/**
 * Size in bytes of a single card bank
 */
#define NESEMU_CARTRIDGE_BANK_SIZE 0x4000 /* 16 KiB */

/**
 * Initial address for the cartridge RAM
 */
#define NESEMU_CARTRIDGE_RAM_BEGIN 0x6000

/**
 * Initial address for the cartridge ROM
 */
#define NESEMU_CARTRIDGE_ROM_BEGIN 0x8000

/**
 * Total size of the addresable space
 */
#define NESEMU_CARTRIDGE_ADDR_SIZE 0xBFE0

/**
 * (∩๏﹏๏)⊃━☆ﾟ.* -- A morsel of black sorcery, woven in the dread tongue of C.
 *
 * Every mapper type is supposed to have its own struct. Each struct is present
 * as a field in `union nes_ines_mapper_t at cartridge.h`. But because you
 * cannot include 'cartridge.h' inside this file (because that file already
 * includes this one), a reference to the mapper struct is given instead
 * 
 * i.e
 * nesemu_mapper_generic_ref_t generic = &(union nes_ines_mapper_t)mapper.nrom;
 *
 * This effectively creates a generic reference to any of the mapper types.
 * Caution. Type safety is now broken!.
 *
 * Because all union members share the same memory, you can grab a reference
 * to any member, but for in order to keep mapper types decoupled an empty type
 * `__self` is included. Use macro `NESEMU_CARTRIDGE_GET_MAPPER_GENERIC_REF`.
 *
 * Now, in order to use this type in each of the callbacks defined by the mapper
 * you should cast this reference to a pointer to the mapper type.
 *
 * i.e
 * (struct nes_ines_nrom_cartridge_t *)generic
 *
 * Make sure to cast to the right type, no type safety here.
 * You can use macro `NESEMU_CARTRIDGE_DEFINE_GENERIC_TYPE` for syntax sugar
 */
typedef void *nesemu_mapper_generic_ref_t;

/**
 * Syntax sugar to create a pointer type variable from a mapper generic reference
 *
 * i.e
 * nesemu_mapper_generic_ref_t self;
 * NESEMU_CARTRIDGE_DEFINE_GENERIC_TYPE(this, struct foo, self);
 *
 * Creates a `this` variable of type `struct foo*`
 *
 * @param name (str) Name of the new variable
 * @param type (str) Type for the new variable (without asterisc)
 * @param self (nesemu_mapper_generic_ref_t) Generic reference
 */
#define NESEMU_CARTRIDGE_DEFINE_GENERIC_TYPE(name, type, self) \
	type *name = (type *)self

/**
 * Function type for a function that reads cartridge data `cdata` of size `len`
 * into the cartridge memory.
 *
 * Should be implemented by each mapper type
 */
typedef nesemu_return_t (*nes_cartridge_loader_t)(
	nesemu_mapper_generic_ref_t self,
	uint8_t *cdata,
	size_t len);

/**
 * Function type for a function that reads from cartridge data
 *
 * Should be implemented by each mapper type.
 *
 * @param content Pointer to an integer where the result will be stored
 */
typedef nesemu_return_t (*nes_cartridge_read_t)(nesemu_mapper_generic_ref_t self,
					       uint16_t addr,
					       uint8_t *content);

/**
 * Function type for a function that writes to cartridge data
 *
 * Should be implemented by each mapper type
 */
typedef nesemu_return_t (*nes_cartridge_write_t)(nesemu_mapper_generic_ref_t self,
						uint16_t addr,
						uint8_t content);

/**
 * Function type for a function that maps `addr` into the appropiate address
 * given cartridge's mapping rules (mirroring).
 *
 * Should be implemented by each mapper type.
 * Reference for implementation:
 * https://www.nesdev.org/wiki/Mirroring
 *
 * Beware!, this function might return `NESEMU_INFO_CARTRIDGE_DELEGATE_RWOP`
 * if the result address lives within a cartridge's internal data structure,
 * indicating that the corresponding r/w operation should be delegated to the
 * cartridge's r/w callbacks.
 *
 * @param mapped Pointer to an integer where the result address will be stored
 *
 * @returns `NESEMU_RETURN_SUCCESS` if mapping was found and valid.
 * `NESEMU_INFO_CARTRIDGE_DELEGATE_RWOP` if the found mapping should be
 * read/written inside the cartridge.
 * Any other negative code if an error ocurred.
 */
typedef nesemu_return_t (*nes_cartridge_mapper_t)(
	nesemu_mapper_generic_ref_t self,
	uint16_t addr,
	uint16_t *mapped);

#endif
