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

void	*put_header(void *ptr, std::size_t size)
{
	t_header_info	*header_info;

	header_info = static_cast<t_header_info *>(ptr);
	header_info->size = size;
	return (header_info + 1);
}

std::size_t	get_header_size(void *ptr)
{
	t_header_info	*header_info;

	header_info = static_cast<t_header_info *>(ptr) - 1;
	return (header_info->size);
}

void	*get_header_pos(void *ptr)
{
	t_header_info	*header_info;

	header_info = static_cast<t_header_info *>(ptr) - 1;
	return (header_info);
}
