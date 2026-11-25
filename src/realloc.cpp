/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:35:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/25 13:25:36 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc_internal.h"

extern "C" void	*realloc(void *ptr, std::size_t size) noexcept
{
	void			*new_ptr;
	std::size_t	old_size;
	std::size_t	i;
	unsigned char	*source;
	unsigned char	*destination;

	if (ptr == nullptr)
		return (malloc(size));

	if (size == 0)
	{
		free(ptr);
		return (nullptr);
	}

	lock_allocator();
	old_size = get_header(ptr)->requested_size;
	if (resize_header_in_place(ptr, size))
	{
		unlock_allocator();
		return (ptr);
	}
	unlock_allocator();

	new_ptr = malloc(size);
	if (new_ptr == nullptr)
		return (nullptr);

	source = static_cast<unsigned char *>(ptr);
	destination = static_cast<unsigned char *>(new_ptr);
	i = 0;
	while (i < old_size)
	{
		destination[i] = source[i];
		i++;
	}
	free(ptr);
	return (new_ptr);
}
