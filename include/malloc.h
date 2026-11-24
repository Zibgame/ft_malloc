/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:15:39 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/24 16:28:41 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <cstddef>

extern "C"
{
	void	*malloc(std::size_t size) noexcept;
	void	free(void *ptr) noexcept;
	void	*realloc(void *ptr, std::size_t size) noexcept;
}

typedef struct alignas(std::max_align_t) s_header_info
{
	std::size_t	size;
	bool   is_free;
} t_header_info;

void			*put_header(void *ptr, std::size_t size);
std::size_t	get_header_size(void *ptr);
void			*get_header_pos(void *ptr);
