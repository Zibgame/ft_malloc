/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:35:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/25 13:33:06 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc_internal.h"

extern "C" void	free(void *ptr) noexcept
{
	t_header_info	*header;
	t_header_info	*zone;
	std::size_t		size;
	t_zone_type		type;

	if (ptr == nullptr)
		return ;

	lock_allocator();
	initialize_debug_options();
	if (is_header_free(ptr))
	{
		unlock_allocator();
		return ;
	}

	header = get_header(ptr);
	size = header->requested_size;
	type = header->type;
	if (is_scribble_enabled())
		scribble_memory(ptr, size, 0x55);
	record_history(HISTORY_FREE, ptr, nullptr, size, type);
	if (release_large_header(header))
	{
		unlock_allocator();
		return ;
	}

	zone = header->zone;
	header->is_free = true;
	header->requested_size = 0;
	merge_free_headers();
	release_empty_zone(zone);
	unlock_allocator();
}
