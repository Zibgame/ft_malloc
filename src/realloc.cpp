/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:35:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/25 13:33:06 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc_internal.h"

extern "C" void	*realloc(void *ptr, std::size_t size) noexcept
{
	void			*new_ptr;
	std::size_t	old_size;
	std::size_t	copy_size;
	std::size_t	i;
	t_zone_type		old_type;
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
	initialize_debug_options();
	old_size = get_header(ptr)->requested_size;
	old_type = get_header(ptr)->type;
	if (resize_header_in_place(ptr, size))
	{
		if (is_scribble_enabled() && size > old_size)
			scribble_memory(static_cast<unsigned char *>(ptr) + old_size,
				size - old_size, 0xAA);
		record_history(HISTORY_REALLOC, ptr, ptr, size, old_type);
		unlock_allocator();
		return (ptr);
	}
	unlock_allocator();

	new_ptr = malloc(size);
	if (new_ptr == nullptr)
		return (nullptr);

	source = static_cast<unsigned char *>(ptr);
	destination = static_cast<unsigned char *>(new_ptr);
	copy_size = old_size;
	if (copy_size > size)
		copy_size = size;
	i = 0;
	while (i < copy_size)
	{
		destination[i] = source[i];
		i++;
	}
	lock_allocator();
	record_history(HISTORY_REALLOC, new_ptr, ptr, size,
		get_header(new_ptr)->type);
	unlock_allocator();
	free(ptr);
	return (new_ptr);
}
