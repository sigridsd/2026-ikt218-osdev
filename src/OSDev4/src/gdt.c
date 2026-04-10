#include "gdt.h"

/*
 * We define three descriptors:
 *   0 - NULL descriptor     (required by CPU, all zeroes)
 *   1 - Kernel Code segment (ring 0, base 0, limit 4 GB)
 *   2 - Kernel Data segment (ring 0, base 0, limit 4 GB)
 *
 * Using a flat memory model: both segments cover the entire 4 GB address space.
 * Segmentation protection is effectively bypassed; paging handles memory later.
 */
#define GDT_ENTRIES 3

/* Access byte bit fields */
#define ACCESS_PRESENT   (1 << 7)   /* Segment is present in memory     */
#define ACCESS_RING0     (0 << 5)   /* Descriptor Privilege Level 0     */
#define ACCESS_SEGMENT   (1 << 4)   /* S=1: code/data (not system)      */
#define ACCESS_EXEC      (1 << 3)   /* Executable (code segment)        */
#define ACCESS_RW        (1 << 1)   /* Readable (code) / Writable (data)*/

/* Flag nibble (upper half of limit_flags byte) */
#define FLAG_GRANULARITY (1 << 7)   /* Limit is in 4 KB pages           */
#define FLAG_32BIT       (1 << 6)   /* 32-bit protected mode segment    */

static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t   gdt_ptr;

/* Defined in gdt_flush.asm - loads GDT and reloads segment registers */
extern void gdt_flush(gdt_ptr_t* ptr);

/*
 * Encodes one GDT entry.
 *   index  - slot in the gdt array (0, 1, 2, ...)
 *   base   - segment base address
 *   limit  - segment limit (20-bit value; combined with granularity flag)
 *   access - access byte (present, DPL, type)
 *   flags  - 4 flag bits in the upper nibble (granularity, size, ...)
 */
static void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t flags)
{
    gdt[index].base_low    =  base        & 0xFFFF;
    gdt[index].base_mid    = (base >> 16) & 0xFF;
    gdt[index].base_high   = (base >> 24) & 0xFF;

    gdt[index].limit_low   =  limit        & 0xFFFF;
    /* High nibble = flags, low nibble = limit bits 16-19 */
    gdt[index].limit_flags = ((limit >> 16) & 0x0F) | (flags & 0xF0);

    gdt[index].access      = access;
}

void gdt_init(void)
{
    /* Entry 0: NULL descriptor - must be all zeroes */
    gdt_set_entry(0, 0, 0, 0, 0);

    /*
     * Entry 1: Kernel Code Segment
     *   Base  = 0x00000000
     *   Limit = 0xFFFFF with 4 KB granularity  =>  covers full 4 GB
     *   Access: Present | Ring 0 | S=1 | Executable | Readable
     *   Flags:  Granularity=1 (4KB pages) | Size=1 (32-bit)
     */
    gdt_set_entry(1,
        0x00000000,
        0xFFFFF,
        ACCESS_PRESENT | ACCESS_RING0 | ACCESS_SEGMENT | ACCESS_EXEC | ACCESS_RW,
        FLAG_GRANULARITY | FLAG_32BIT);

    /*
     * Entry 2: Kernel Data Segment
     *   Same base/limit as code; differs only in the access byte (no execute).
     *   Access: Present | Ring 0 | S=1 | Writable
     */
    gdt_set_entry(2,
        0x00000000,
        0xFFFFF,
        ACCESS_PRESENT | ACCESS_RING0 | ACCESS_SEGMENT | ACCESS_RW,
        FLAG_GRANULARITY | FLAG_32BIT);

    /* Fill the GDT pointer struct */
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (uint32_t)&gdt;

    /* Tell the CPU about the new GDT and reload segment registers */
    gdt_flush(&gdt_ptr);
}
