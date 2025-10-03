# 👾 libnesemu
An embedded friendly NES emulation library built in good 'ol C!

## Usage
```c
/* Create the cartridge structure */
struct nes_cartridge_t cartridge;

long int clen = /* Size of the raw bytes array */;
char *cdata = /* Cartridge raw bytes read from any source */;

/* Allocate cartridge internal data from bytes */
nesemu_return_t err = nes_cartridge_read_ines(&cartridge, cdata, clen);
```

```c
/* Initialize memory bus */
struct nes_main_memory_t mem;
nesemu_return_t err = nes_mem_init(&mem, &cartridge);
```

```c
/* Initialize CPU */
struct nes_cpu_t cpu;
nesemu_return_t err = nes_cpu_init(&cpu, &mem);
```
