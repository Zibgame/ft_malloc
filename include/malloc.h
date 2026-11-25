#pragma once

#include <cstddef>

extern "C"
{
	void	*malloc(std::size_t size) noexcept;
	void	free(void *ptr) noexcept;
	void	*realloc(void *ptr, std::size_t size) noexcept;
	void	show_alloc_mem() noexcept;
}
