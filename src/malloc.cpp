/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:27:44 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/25 13:33:06 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc_internal.h"
#include <cstdlib>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct s_allocator_state
{
	t_header_info		*header;
	pthread_mutex_t		mutex;
	t_history_event		history[256];
	std::size_t			history_count;
	std::size_t			history_next;
	std::size_t			next_event_id;
	bool				options_initialized;
	bool				debug_enabled;
	bool				scribble_enabled;
} t_allocator_state;

static t_allocator_state	g_state = {
	nullptr, PTHREAD_MUTEX_INITIALIZER, {}, 0, 0, 1, false, false, false
};

static const std::size_t	TINY_MAX = 128;
static const std::size_t	SMALL_MAX = 1024;
static const std::size_t	BLOCKS_PER_ZONE = 100;

void	lock_allocator()
{
	pthread_mutex_lock(&g_state.mutex);
}

void	unlock_allocator()
{
	pthread_mutex_unlock(&g_state.mutex);
}

t_header_info	*get_first_header()
{
	return (g_state.header);
}

static bool	environment_enabled(const char *name)
{
	const char	*value;

	value = std::getenv(name);
	if (value == nullptr || value[0] == '\0')
		return (false);

	return (!(value[0] == '0' && value[1] == '\0'));
}

void	initialize_debug_options()
{
	if (g_state.options_initialized)
		return ;

	g_state.debug_enabled = environment_enabled("FT_MALLOC_DEBUG");
	g_state.scribble_enabled = environment_enabled("FT_MALLOC_SCRIBBLE");
	g_state.options_initialized = true;
}

bool	is_debug_enabled()
{
	return (g_state.debug_enabled);
}

bool	is_scribble_enabled()
{
	return (g_state.scribble_enabled);
}

void	scribble_memory(void *ptr, std::size_t size, unsigned char value)
{
	unsigned char	*memory;
	std::size_t		i;

	memory = static_cast<unsigned char *>(ptr);
	i = 0;
	while (i < size)
	{
		memory[i] = value;
		i++;
	}
}

void	record_history(t_history_action action, void *ptr,
	void *old_ptr, std::size_t size, t_zone_type type)
{
	t_history_event	*event;

	event = &g_state.history[g_state.history_next];
	event->id = g_state.next_event_id++;
	event->action = action;
	event->ptr = ptr;
	event->old_ptr = old_ptr;
	event->size = size;
	event->type = type;
	g_state.history_next = (g_state.history_next + 1) % 256;
	if (g_state.history_count < 256)
		g_state.history_count++;

	if (g_state.debug_enabled)
		write_debug_event(event);
}

std::size_t	get_history_count()
{
	return (g_state.history_count);
}

const t_history_event	*get_history_event(std::size_t index)
{
	std::size_t	first;
	std::size_t	position;

	if (index >= g_state.history_count)
		return (nullptr);

	first = 0;
	if (g_state.history_count == 256)
		first = g_state.history_next;
	position = (first + index) % 256;
	return (&g_state.history[position]);
}

void	merge_free_headers()
{
	t_header_info	*header;
	char			*end;

	header = g_state.header;
	while (header != nullptr && header->next != nullptr)
	{
		end = reinterpret_cast<char *>(header + 1) + header->size;
		if (header->is_free && header->next->is_free
			&& header->type == header->next->type
			&& header->zone == header->next->zone
			&& end == reinterpret_cast<char *>(header->next))
			merge_next_free_header(header);
		else
			header = header->next;
	}
}

static std::size_t	align_size(std::size_t size)
{
	const std::size_t alignment = alignof(std::max_align_t);

	if (size > static_cast<std::size_t>(-1) - (alignment - 1))
		return (0);

	return ((size + alignment - 1) / alignment * alignment);
}

static t_zone_type	get_zone_type(std::size_t size)
{
	if (size <= TINY_MAX)
		return (TINY);

	if (size <= SMALL_MAX)
		return (SMALL);

	return (LARGE);
}

static std::size_t	get_mapping_size(std::size_t size, t_zone_type type)
{
	long			page_size;
	std::size_t	required_size;
	std::size_t	block_size;
	std::size_t	page;

	page_size = sysconf(_SC_PAGESIZE);
	if (page_size <= 0)
		return (0);

	page = static_cast<std::size_t>(page_size);
	if (type == TINY)
		block_size = TINY_MAX;
	else if (type == SMALL)
		block_size = SMALL_MAX;
	else
		block_size = size;
	if (block_size > static_cast<std::size_t>(-1)
		- sizeof(t_header_info))
		return (0);

	required_size = sizeof(t_header_info) + block_size;
	if (type != LARGE)
	{
		if (required_size > static_cast<std::size_t>(-1) / BLOCKS_PER_ZONE)
			return (0);

		required_size *= BLOCKS_PER_ZONE;
	}

	if (required_size > static_cast<std::size_t>(-1) - (page - 1))
		return (0);

	return ((required_size + page - 1) / page * page);
}

static t_header_info	*allocate_zone(std::size_t size, t_zone_type type)
{
	std::size_t	mapping_size;
	void			*ptr;
	t_header_info	*header;

	mapping_size = get_mapping_size(size, type);
	if (mapping_size == 0)
		return (nullptr);

	ptr = mmap(nullptr, mapping_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ptr == MAP_FAILED)
		return (nullptr);

	header = static_cast<t_header_info *>(ptr);
	header->size = mapping_size - sizeof(t_header_info);
	header->requested_size = 0;
	header->mapping_size = mapping_size;
	header->is_free = true;
	header->type = type;
	header->next = g_state.header;
	header->zone = header;
	g_state.header = header;
	return (header);
}

static t_header_info	*find_free_header(std::size_t size, t_zone_type type)
{
	t_header_info	*header;

	header = g_state.header;
	while (header != nullptr)
	{
		if (header->is_free && header->type == type && header->size >= size)
			return (header);

		header = header->next;
	}

	return (nullptr);
}

static void	split_header(t_header_info *header, std::size_t size)
{
	t_header_info	*old_next;
	t_header_info	*next_header;
	char			*next_position;

	if (header->size < size + sizeof(t_header_info) + alignof(std::max_align_t))
		return ;

	old_next = header->next;
	next_position = reinterpret_cast<char *>(header + 1) + size;
	next_header = reinterpret_cast<t_header_info *>(next_position);
	if (old_next == next_header)
		old_next = old_next->next;

	next_header->size = header->size - size - sizeof(t_header_info);
	next_header->requested_size = 0;
	next_header->mapping_size = 0;
	next_header->is_free = true;
	next_header->type = header->type;
	next_header->next = old_next;
	next_header->zone = header->zone;
	header->size = size;
	header->next = next_header;
}

bool	release_large_header(t_header_info *header)
{
	t_header_info	*current;
	t_header_info	*previous;

	if (header->type != LARGE)
		return (false);

	current = g_state.header;
	previous = nullptr;
	while (current != nullptr && current != header)
	{
		previous = current;
		current = current->next;
	}
	if (current == nullptr)
		return (false);

	if (previous == nullptr)
		g_state.header = current->next;
	else
		previous->next = current->next;

	munmap(current, current->mapping_size);
	return (true);
}

static std::size_t	count_zones(t_zone_type type)
{
	t_header_info	*header;
	std::size_t	count;

	header = g_state.header;
	count = 0;
	while (header != nullptr)
	{
		if (header->zone == header && header->type == type)
			count++;

		header = header->next;
	}

	return (count);
}

void	release_empty_zone(t_header_info *zone)
{
	t_header_info	*after_zone;
	t_header_info	*current;
	t_header_info	*previous;

	if (zone == nullptr || zone->type == LARGE || !zone->is_free)
		return ;

	current = zone;
	while (current != nullptr && current->zone == zone)
	{
		if (!current->is_free)
			return ;

		current = current->next;
	}
	after_zone = current;

	if (count_zones(zone->type) <= 1)
	{
		zone->size = zone->mapping_size - sizeof(t_header_info);
		zone->requested_size = 0;
		zone->next = after_zone;
		return ;
	}

	current = g_state.header;
	previous = nullptr;
	while (current != nullptr && current != zone)
	{
		previous = current;
		current = current->next;
	}

	if (current == nullptr)
		return ;

	if (previous == nullptr)
		g_state.header = after_zone;
	else
		previous->next = after_zone;

	munmap(current, current->mapping_size);
}

bool	resize_header_in_place(void *ptr, std::size_t size)
{
	t_header_info	*header;
	t_header_info	*next;
	std::size_t	aligned_size;
	char			*end;

	header = get_header(ptr);
	aligned_size = align_size(size);
	if (aligned_size == 0 || get_zone_type(aligned_size) != header->type)
		return (false);

	if (header->size >= aligned_size)
	{
		if (header->type != LARGE)
			split_header(header, aligned_size);

		header->requested_size = size;
		merge_free_headers();
		return (true);
	}

	next = header->next;
	end = reinterpret_cast<char *>(header + 1) + header->size;
	if (header->type == LARGE || next == nullptr || !next->is_free
		|| next->zone != header->zone
		|| end != reinterpret_cast<char *>(next)
		|| header->size + sizeof(t_header_info) + next->size < aligned_size)
		return (false);

	header->size += sizeof(t_header_info) + next->size;
	header->next = next->next;
	split_header(header, aligned_size);
	header->requested_size = size;
	return (true);
}

extern "C" void	*malloc(std::size_t size) noexcept
{
	std::size_t	aligned_size;
	t_header_info	*header;
	t_zone_type		type;
	void			*result;

	if (size == 0)
		return (nullptr);

	lock_allocator();
	initialize_debug_options();
	aligned_size = align_size(size);
	if (aligned_size == 0)
	{
		unlock_allocator();
		return (nullptr);
	}

	type = get_zone_type(aligned_size);
	header = find_free_header(aligned_size, type);
	if (header == nullptr)
		header = allocate_zone(aligned_size, type);

	if (header == nullptr)
	{
		unlock_allocator();
		return (nullptr);
	}

	if (type != LARGE)
		split_header(header, aligned_size);

	header->requested_size = size;
	header->is_free = false;
	result = header + 1;
	if (is_scribble_enabled())
		scribble_memory(result, size, 0xAA);
	record_history(HISTORY_MALLOC, result, nullptr, size, type);
	unlock_allocator();
	return (result);
}
