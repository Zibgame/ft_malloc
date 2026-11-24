/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:27:44 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/24 15:55:31 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"
#include <sys/mman.h>

static void	*allocate_memory(std::size_t size)
{
	void	*ptr;

	ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ptr == MAP_FAILED)
		return (nullptr);
	return (ptr);
}

extern "C" void	*malloc(std::size_t size) noexcept
{
	std::size_t	mapping_size;
	void			*ptr;

	if (size == 0)
		return (nullptr);

	if (size > static_cast<std::size_t>(-1) - sizeof(t_header_info))
		return (nullptr);

	mapping_size = sizeof(t_header_info) + size;
	ptr = allocate_memory(mapping_size);

	if (ptr == nullptr)
		return (nullptr);

	return (put_header(ptr, size));
}
