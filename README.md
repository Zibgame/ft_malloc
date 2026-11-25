# ft_malloc

A small dynamic memory allocator written in C++ for the 42 `malloc` project.
It provides replacements for `malloc`, `free`, and `realloc` using only
page-backed memory obtained through `mmap`.

## Features

- Aligned allocations suitable for standard C and C++ types
- `TINY`, `SMALL`, and `LARGE` allocation classes
- Preallocated `TINY` and `SMALL` zones with room for at least 100 blocks
- Reuse and coalescing of adjacent free blocks
- In-place `realloc` growth when the following block is available
- Dedicated mappings for `LARGE` allocations
- Release of excess empty shared zones
- Thread-safe allocator state protected by a mutex
- Allocation inspection through `show_alloc_mem()`
- Extended zone, block, hexadecimal, and history output
- Debug logging and memory scribbling through environment variables
- Overflow checks for size and page-rounding calculations

## Allocation classes

| Class | Requested size | Strategy |
| --- | ---: | --- |
| `TINY` | 1–128 bytes | Shared preallocated zone |
| `SMALL` | 129–1024 bytes | Shared preallocated zone |
| `LARGE` | More than 1024 bytes | Dedicated `mmap` mapping |

Each allocated block is preceded by an aligned header containing its capacity,
requested size, state, class, and a link to the next block.

```text
[ header ][ user memory ][ header ][ user memory ] ...
```

## Project layout

```text
include/malloc.h           Public API
include/malloc_internal.h  Allocator structures and internal helpers
src/malloc.cpp             Allocation and zone management
src/free.cpp               Block release
src/realloc.cpp            Block resizing
src/header.cpp             Header access and coalescing
src/show.cpp               Allocation display
src/debug.cpp              Debug event logging
test/main.cpp              Complete test scenarios
```

## Build

```sh
make
```

The build creates the host-specific library required by the subject and a
portable symbolic link:

```text
libft_malloc_<HOSTTYPE>.so
libft_malloc.so -> libft_malloc_<HOSTTYPE>.so
```

## Test

```sh
make test
```

The test suite checks alignment, allocation classes, in-place and moving
`realloc`, data preservation, zero-size behavior, multiple shared zones,
concurrent allocations from four threads, and `show_alloc_mem()`.

The allocator can also be injected into another program:

```sh
LD_PRELOAD="$PWD/libft_malloc.so" ./your_program
```

## Public API

```cpp
void  *malloc(std::size_t size) noexcept;
void   free(void *ptr) noexcept;
void  *realloc(void *ptr, std::size_t size) noexcept;
void   show_alloc_mem() noexcept;
void   show_alloc_mem_ex() noexcept;
```

## Debug tools

`show_alloc_mem_ex()` displays every mapped zone, used and free blocks,
requested and available capacities, up to 32 bytes of hexadecimal data per
active block, and the 32 most recent allocation events.

The allocator stores the latest 256 events in a fixed circular history. This
history does not call `malloc` and is safe to inspect while the allocator mutex
is held.

Enable allocation logs on standard error:

```sh
FT_MALLOC_DEBUG=1 LD_PRELOAD="$PWD/libft_malloc.so" ./your_program
```

Fill newly allocated bytes with `0xAA` and released bytes with `0x55`:

```sh
FT_MALLOC_SCRIBBLE=1 LD_PRELOAD="$PWD/libft_malloc.so" ./your_program
```

Both variables can be enabled together.

## Bonus status

Thread safety, block coalescing, empty-zone release, in-place `realloc`, extended
allocation display, debug environment variables, and allocation history are
implemented.
