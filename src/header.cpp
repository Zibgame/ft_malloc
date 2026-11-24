/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   header.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 16:30:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/24 16:28:12 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

void	*put_header(void *ptr, std::size_t size, t_zone_type type)
{
	t_header_info	*header_info;

	header_info = static_cast<t_header_info *>(ptr);
	header_info->size = size;
	header_info->requested_size = size;
	header_info->is_free = false;
	header_info->type = type;
	header_info->next = nullptr;
	return (header_info + 1);
}

std::size_t	get_header_size(void *ptr)
{
	t_header_info	*header_info;

	header_info = static_cast<t_header_info *>(ptr) - 1;
	return (header_info->size);
}

t_header_info	*get_header(void *ptr)
{
	t_header_info	*header_info;

	header_info = static_cast<t_header_info *>(ptr) - 1;
	return (header_info);
}

void	*get_header_pos(void *ptr)
{
	return (get_header(ptr));
}

bool	is_header_free(void *ptr)
{
	return (get_header(ptr)->is_free);
}

void	merge_next_free_header(t_header_info *header)
{
	t_header_info	*next;
	char			*end;

	next = header->next;
	if (next == nullptr || !header->is_free || !next->is_free)
		return ;

	if (header->type != next->type)
		return ;

	end = reinterpret_cast<char *>(header + 1) + header->size;
	if (end != reinterpret_cast<char *>(next))
		return ;

	header->size += sizeof(t_header_info) + next->size;
	header->next = next->next;
}
