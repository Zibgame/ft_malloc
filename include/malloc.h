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
	void	show_alloc_mem() noexcept;
}

typedef enum e_zone_type
{
	TINY,
	SMALL,
	LARGE
} t_zone_type;

typedef struct alignas(std::max_align_t) s_header_info
{
	std::size_t			size;
	std::size_t			requested_size;
	bool				is_free;
	t_zone_type			type;
	struct s_header_info	*next;
} t_header_info;

void			*put_header(void *ptr, std::size_t size, t_zone_type type);
std::size_t	get_header_size(void *ptr);
void			*get_header_pos(void *ptr);
t_header_info	*get_header(void *ptr);
bool			is_header_free(void *ptr);
void			merge_next_free_header(t_header_info *header);
void			merge_free_headers();
bool			release_large_header(t_header_info *header);
