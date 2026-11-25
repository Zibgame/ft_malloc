/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc_internal.h                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 13:25:36 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/25 13:33:06 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "malloc.h"

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
	std::size_t			mapping_size;
	bool				is_free;
	t_zone_type			type;
	struct s_header_info	*next;
	struct s_header_info	*zone;
} t_header_info;

typedef enum e_history_action
{
	HISTORY_MALLOC,
	HISTORY_FREE,
	HISTORY_REALLOC
} t_history_action;

typedef struct s_history_event
{
	std::size_t		id;
	t_history_action	action;
	void			*ptr;
	void			*old_ptr;
	std::size_t		size;
	t_zone_type		type;
} t_history_event;

void			*put_header(void *ptr, std::size_t size, t_zone_type type);
std::size_t		get_header_size(void *ptr);
void			*get_header_pos(void *ptr);
t_header_info	*get_header(void *ptr);
bool			is_header_free(void *ptr);
void			merge_next_free_header(t_header_info *header);
void			merge_free_headers();
bool			release_large_header(t_header_info *header);
void			release_empty_zone(t_header_info *zone);
bool			resize_header_in_place(void *ptr, std::size_t size);
void			lock_allocator();
void			unlock_allocator();
t_header_info	*get_first_header();
void			initialize_debug_options();
bool			is_debug_enabled();
bool			is_scribble_enabled();
void			scribble_memory(void *ptr, std::size_t size, unsigned char value);
void			record_history(t_history_action action, void *ptr,
					void *old_ptr, std::size_t size, t_zone_type type);
std::size_t		get_history_count();
const t_history_event	*get_history_event(std::size_t index);
void			write_debug_event(const t_history_event *event);
