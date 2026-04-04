# Assignment 4: Memory and PIT

## 1. Introduction

This assignment extended the active `3_jakub` kernel with two fundamental subsystems: kernel memory management and a programmable interval timer (PIT) driver. The goal was to make dynamic allocation available inside the kernel, enable paging, and provide timing primitives that could later support more advanced functionality such as input handling, sleep operations, and hardware-driven features.

The work was not limited to copying starter code. The attached assignment material was reviewed, adapted to the local codebase, and then integrated into the active kernel structure. Later, the architecture was cleaned up further so that the code is easier to maintain and easier for the next developer to continue from.

## 2. Active Architecture After Cleanup

A key result of the final cleanup is that the project now has a clear separation between active kernel code and reference material.

### 2.1 Active Kernel Code

The live implementation now uses the following files:

- `include/kernel/heap.h`
- `include/kernel/paging.h`
- `include/kernel/pit.h`
- `include/libc/memory.h`
- `src/memory/heap.c`
- `src/memory/paging.c`
- `src/libc/memory.c`
- `src/pit.c`

This layout is more professional than the earlier structure, because heap management, paging, timer logic, and libc-style memory helpers now have separate responsibilities.

### 2.2 Reference Material

The original attached files are no longer stored under `include/`, because that made them look like active kernel headers. They were moved into:

- `reference/assignment4/`

This directory is intentionally not part of the live build. It exists only for traceability and comparison against the assignment templates.

The most relevant reference files are now:

- `reference/assignment4/kernel/include/kernel/memory.h`
- `reference/assignment4/kernel/include/kernel/pit.h`
- `reference/assignment4/kernel/src/memory/malloc.c`
- `reference/assignment4/kernel/src/memory/memory.c`
- `reference/assignment4/kernel/src/memory/memutils.c`
- `reference/assignment4/apps/song/src/song.cpp`

### 2.3 Metadata Cleanup

All `Zone.Identifier` files were removed. These files were Windows download metadata, not source code, and had no legitimate role in the repository.

## 3. Memory Subsystem

### 3.1 Heap Interface and Implementation

The active heap API is declared in `include/kernel/heap.h` and implemented in `src/memory/heap.c`.

The heap implementation provides:

- `init_kernel_memory(uint32_t *kernel_end)`
- `malloc(size_t size)`
- `free(void *mem)`
- `pmalloc(size_t size)`
- `pfree(void *mem)`
- `print_memory_layout(void)`

The heap allocator is a simple first-fit design using an allocation header with:

- a `status` field indicating whether the block is in use
- a `size` field describing the payload size

The allocator supports:

- dynamic allocation and freeing
- reuse of freed blocks
- splitting of larger free blocks
- coalescing of adjacent free blocks
- zero-initialization of newly allocated memory

The heap starts at the first page-aligned address after the kernel image and grows upward until the reserved page-aligned region near `0x400000`.

### 3.2 Page-Aligned Allocation

The page-aligned allocator is also implemented in `src/memory/heap.c`.

This part of the allocator is separate from ordinary `malloc()` because paging-related data structures and some future low-level features may need page-sized or page-aligned memory regions.

The implementation uses a small descriptor array and supports contiguous multi-page allocation, which is a meaningful improvement over the simpler single-page assumption seen in the reference material.

### 3.3 Paging

The paging interface is declared in `include/kernel/paging.h` and implemented in `src/memory/paging.c`.

The current paging setup is intentionally simple:

- the page directory is placed at `0x400000`
- the first page table starts at `0x401000`
- the first 8 MB are identity-mapped

This is sufficient for the current kernel because:

- the kernel is loaded at 1 MB
- the general heap remains below the page-directory region
- the paging data structures are placed in a known location

The function `init_paging()` initializes the page directory and page tables, then enables paging by loading `cr3` and setting the paging bit in `cr0`.

### 3.4 Memory Utility Functions

General memory helper functions are now declared in `include/libc/memory.h` and implemented in `src/libc/memory.c`.

These functions are:

- `memcpy`
- `memset`
- `memset16`

This was an important cleanup step. These functions are generic libc-style routines, so they belong more naturally in `libc` than in the heap or paging modules.

### 3.5 Compatibility Layer

To avoid breaking older includes during the cleanup, `include/kernel/memory.h` was kept as a compatibility umbrella header.

That header now forwards to:

- `kernel/heap.h`
- `kernel/paging.h`
- `libc/memory.h`

This means the architecture is cleaner, while the transition remains safe.

## 4. PIT Subsystem

The PIT API is declared in `include/kernel/pit.h` and implemented in `src/pit.c`.

The current implementation provides:

- `init_pit()`
- `sleep_busy(uint32_t milliseconds)`
- `sleep_interrupt(uint32_t milliseconds)`
- `pit_get_ticks()`

The timer is configured to 1000 Hz on PIT channel 0. This gives a simple model where one tick corresponds approximately to one millisecond.

The PIT driver is integrated with the existing interrupt system by registering an IRQ0 handler. The IRQ0 callback increments a global tick counter, which is then used by both sleep functions.

The behavior of the two sleep functions differs intentionally:

- `sleep_busy()` spins until the requested number of ticks has passed
- `sleep_interrupt()` uses `sti` and `hlt` inside the wait loop so the CPU sleeps until the next interrupt arrives

This matches the assignment requirement and also creates a useful timing primitive for future work.

## 5. Kernel Integration

The startup path in `src/kernel.c` now initializes the core subsystems in this order:

1. terminal initialization
2. GDT
3. IDT
4. ISR and IRQ setup
5. heap initialization
6. paging initialization
7. keyboard initialization
8. PIT initialization
9. enabling CPU interrupts

The linker-provided symbol `end` is still used as the boundary between the kernel image and the heap start.

The C++ side in `src/kernel.cpp` now provides global `operator new` and `operator delete` overloads that forward to the kernel heap. That means both C and C++ code can use the same memory subsystem consistently.

## 6. User-Visible Proof of Functionality

The final system does not rely only on silent initialization. Several visible runtime behaviors demonstrate that the implementation is active.

### 6.1 Memory Proof

Memory use is demonstrated in two ways:

- the C++ `new` path is exercised in `src/kernel.cpp`
- the CLI history system in `src/interrupts/keyboard.c` stores entered lines on the heap using `malloc()` and later frees them again

This makes the allocator observable during normal interaction rather than only during boot.

### 6.2 PIT Proof

The PIT driver exposes `pit_get_ticks()`, and the CLI includes commands such as `ticks` and `uptime`. These commands provide direct evidence that IRQ0 is firing and that the kernel timer is progressing.

### 6.3 CLI Integration

The keyboard path evolved into a simple CLI that helps demonstrate the kernel state. Relevant commands include:

- `help`
- `meminfo`
- `history`
- `clearhistory`
- `ticks`
- `uptime`
- `clear`

This CLI was not required by the assignment, but it substantially improved observability and made debugging easier.

## 7. Stability and UX Fixes During Integration

The final result required several debugging and refinement steps.

### 7.1 Boot Fallthrough and Page Faults

At one point the kernel returned from `main()` and fell through into unintended code in `_start`. That caused repeated page faults.

The fix was to add a proper idle loop in `src/multiboot2.asm` after `call main`.

### 7.2 Interrupt-Friendly Idle Behavior

A later version of the idle loop mistakenly disabled interrupts before halting, which stopped keyboard input from working. This was corrected by using an interrupt-friendly `sti` + `hlt` loop.

### 7.3 Terminal Initialization and Clear-Screen Behavior

The VGA text output path originally could leave stale screen data visible. The terminal was updated so that boot starts from a clean screen state, and later the terminal gained software scrollback and proper cursor synchronization.

### 7.4 Cursor Synchronization

The visual cursor shown by QEMU did not initially match the kernel’s logical cursor position. The terminal code in `src/libc/stdio.c` now updates the VGA hardware cursor explicitly, which makes the CLI feel much more consistent.

## 8. Architectural Cleanup After the Assignment Work

After the functionality was working, the codebase was cleaned up so that the structure better reflects real responsibilities.

The most important cleanup steps were:

- moving reference files out of `include/` and into `reference/assignment4/`
- deleting all `Zone.Identifier` files
- renaming `malloc.c` to `heap.c`
- renaming `memory.c` to `paging.c`
- moving `memutils.c` into `src/libc/memory.c`
- splitting the public headers into `heap.h`, `paging.h`, and `libc/memory.h`
- keeping `kernel/memory.h` as a compatibility header instead of an overloaded catch-all API

This cleanup did not change the overall behavior of the kernel, but it made the design easier to read and much easier to hand over.

## 9. Readiness for Assignment 5: Music Player

### 9.1 Verdict

Yes. The project is now ready for Assignment 5 at the infrastructure level.

That does not mean the music player is already implemented. It means the missing work is now the actual sound feature itself, not missing prerequisites.

### 9.2 Why the Project Is Ready

The following prerequisites are already in place:

- low-level port I/O through `common.h` and `src/common.c`
- PIT constants in `include/kernel/pit.h`
- access to PIT channel 2 and `PC_SPEAKER_PORT` through that same header
- a working timer on PIT channel 0 in `src/pit.c`
- interrupt-driven waiting through `sleep_interrupt()`
- working heap allocation and C++ `new` for player objects or song structures
- a functioning build system in `CMakeLists.txt` for both C and C++ kernel code

This is exactly the groundwork a PC speaker music player needs.

### 9.3 What the Next Developer Still Needs to Implement

The next developer still needs to add the actual Assignment 5 functionality:

- speaker enable/disable logic using port `0x61`
- note playback by programming PIT channel 2
- a small song representation such as `Note`, `Song`, and `SongPlayer`
- a playback loop that calls `sleep_interrupt()` between notes
- CMake entries for any new source files

In other words, the remaining work is feature implementation, not kernel foundation work.

### 9.4 Important Guidance for the Next Developer

There is one architectural rule that matters for Assignment 5:

- keep PIT channel 0 reserved for the kernel timer
- use PIT channel 2 for sound generation

That separation is already reflected in `include/kernel/pit.h`, and it allows the timer and speaker to coexist correctly.

The reference song example in `reference/assignment4/apps/song/src/song.cpp` is useful as inspiration, but it should not be copied blindly. It still reflects an older header layout and should be adapted to the active codebase.

## 10. Validation

The refactored project was validated by rebuilding the kernel and recreating the bootable image.

Successful commands:

```bash
cmake --build build/3_jakub -j4
cmake --build build/3_jakub --target uiaos-create-image -j2
```

This confirmed that:

- the renamed source files are wired correctly into the build
- the cleaned header structure is valid
- the kernel still links correctly
- the ISO creation flow still works after the architecture cleanup

## 11. Conclusion

Assignment 4 is complete, integrated, and now better structured than a direct starter-code import would have been. The memory subsystem, paging subsystem, and PIT subsystem are all active in the real kernel, and the codebase has been cleaned so that the next developer can navigate it without having to guess which files are live and which files are only references.

Most importantly for the handover: the kernel is now a reasonable foundation for Assignment 5. The next developer does not need to fix memory, paging, timer setup, or repository structure before starting the music player. The next step is to build the PC speaker playback feature itself on top of the foundation that is now in place.
