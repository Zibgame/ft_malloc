/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:35:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/24 16:22:48 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

extern "C" void	free(void *ptr) noexcept
{
	t_header_info	*header;
	t_header_info	*zone;

	if (ptr == nullptr)
		return ;

	lock_allocator();
	if (is_header_free(ptr))
	{
		unlock_allocator();
		return ;
	}

	header = get_header(ptr);
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
