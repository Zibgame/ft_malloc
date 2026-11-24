/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:27:44 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/24 17:48:02 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"
#include <cstdint>
#include <sys/mman.h>
#include <unistd.h>

static t_header_info	*g_first_header = nullptr;

static const std::size_t	TINY_MAX = 128;
static const std::size_t	SMALL_MAX = 1024;
static const std::size_t	BLOCKS_PER_ZONE = 100;

void	merge_free_headers()
{
	t_header_info	*header;
	char			*end;

	header = g_first_header;
	while (header != nullptr && header->next != nullptr)
	{
		end = reinterpret_cast<char *>(header + 1) + header->size;
		if (header->is_free && header->next->is_free
			&& header->type == header->next->type
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
	header->is_free = true;
	header->type = type;
	header->next = g_first_header;
	g_first_header = header;
	return (header);
}

static t_header_info	*find_free_header(std::size_t size, t_zone_type type)
{
	t_header_info	*header;

	header = g_first_header;
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
	t_header_info	*next_header;
	char			*next_position;

	if (header->size < size + sizeof(t_header_info) + alignof(std::max_align_t))
		return ;

	next_position = reinterpret_cast<char *>(header + 1) + size;
	next_header = reinterpret_cast<t_header_info *>(next_position);
	next_header->size = header->size - size - sizeof(t_header_info);
	next_header->requested_size = 0;
	next_header->is_free = true;
	next_header->type = header->type;
	next_header->next = header->next;
	header->size = size;
	header->next = next_header;
}

bool	release_large_header(t_header_info *header)
{
	t_header_info	*current;
	t_header_info	*previous;
	std::size_t	mapping_size;

	if (header->type != LARGE)
		return (false);

	current = g_first_header;
	previous = nullptr;
	while (current != nullptr && current != header)
	{
		previous = current;
		current = current->next;
	}
	if (current == nullptr)
		return (false);

	if (previous == nullptr)
		g_first_header = current->next;
	else
		previous->next = current->next;

	mapping_size = sizeof(t_header_info) + current->size;
	munmap(current, mapping_size);
	return (true);
}

extern "C" void	*malloc(std::size_t size) noexcept
{
	std::size_t	aligned_size;
	t_header_info	*header;
	t_zone_type		type;

	if (size == 0)
		return (nullptr);

	aligned_size = align_size(size);
	if (aligned_size == 0)
		return (nullptr);

	type = get_zone_type(aligned_size);
	header = find_free_header(aligned_size, type);
	if (header == nullptr)
		header = allocate_zone(aligned_size, type);

	if (header == nullptr)
		return (nullptr);

	if (type != LARGE)
		split_header(header, aligned_size);

	header->requested_size = size;
	header->is_free = false;
	return (header + 1);
}

static void	write_text(const char *text)
{
	std::size_t	length;

	length = 0;
	while (text[length] != '\0')
		length++;

	write(STDOUT_FILENO, text, length);
}

static void	write_number(std::size_t number, const char *digits)
{
	char		buffer[32];
	std::size_t	base;
	std::size_t	i;

	base = 0;
	while (digits[base] != '\0')
		base++;

	i = sizeof(buffer);
	if (number == 0)
		buffer[--i] = '0';

	while (number != 0)
	{
		buffer[--i] = digits[number % base];
		number /= base;
	}
	write(STDOUT_FILENO, buffer + i, sizeof(buffer) - i);
}

static t_header_info	*next_header_by_address(t_zone_type type,
	std::uintptr_t previous_address)
{
	t_header_info	*header;
	t_header_info	*next;
	std::uintptr_t	address;

	header = g_first_header;
	next = nullptr;
	while (header != nullptr)
	{
		address = reinterpret_cast<std::uintptr_t>(header);
		if (header->type == type && address > previous_address
			&& (next == nullptr || address
				< reinterpret_cast<std::uintptr_t>(next)))
			next = header;

		header = header->next;
	}

	return (next);
}

static std::size_t	show_zone_type(t_zone_type type, const char *name)
{
	t_header_info	*header;
	std::uintptr_t	address;
	std::uintptr_t	start;
	std::size_t	total;

	header = next_header_by_address(type, 0);
	if (header == nullptr)
		return (0);

	write_text(name);
	write_text(" : 0x");
	write_number(reinterpret_cast<std::uintptr_t>(header), "0123456789ABCDEF");
	write_text("\n");
	total = 0;
	while (header != nullptr)
	{
		address = reinterpret_cast<std::uintptr_t>(header);
		if (!header->is_free)
		{
			start = reinterpret_cast<std::uintptr_t>(header + 1);
			write_text("0x");
			write_number(start, "0123456789ABCDEF");
			write_text(" - 0x");
			write_number(start + header->requested_size,
				"0123456789ABCDEF");
			write_text(" : ");
			write_number(header->requested_size, "0123456789");
			write_text(" bytes\n");
			total += header->requested_size;
		}

		header = next_header_by_address(type, address);
	}

	return (total);
}

extern "C" void	show_alloc_mem() noexcept
{
	std::size_t	total;

	total = show_zone_type(TINY, "TINY");
	total += show_zone_type(SMALL, "SMALL");
	total += show_zone_type(LARGE, "LARGE");
	write_text("Total : ");
	write_number(total, "0123456789");
	write_text(" bytes\n");
}
