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
```

## Current scope

The mandatory allocation behavior and thread-safety bonus are implemented.
Extended diagnostics such as `show_alloc_mem_ex()` and malloc debug environment
variables are not included.
