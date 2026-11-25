/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   show.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 13:13:30 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/25 13:33:06 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc_internal.h"
#include <cstdint>
#include <unistd.h>

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

static t_header_info	*next_zone_by_address(t_zone_type type,
	std::uintptr_t previous_address)
{
	t_header_info	*header;
	t_header_info	*next_zone;
	std::uintptr_t	address;

	header = get_first_header();
	next_zone = nullptr;
	while (header != nullptr)
	{
		address = reinterpret_cast<std::uintptr_t>(header);
		if (header->zone == header && header->type == type
			&& address > previous_address
			&& (next_zone == nullptr || address
				< reinterpret_cast<std::uintptr_t>(next_zone)))
			next_zone = header;

		header = header->next;
	}

	return (next_zone);
}

static const char	*action_name(t_history_action action)
{
	if (action == HISTORY_MALLOC)
		return ("malloc");
	if (action == HISTORY_FREE)
		return ("free");
	return ("realloc");
}

static void	write_pointer(const void *ptr)
{
	write_text("0x");
	write_number(reinterpret_cast<std::uintptr_t>(ptr), "0123456789ABCDEF");
}

static void	write_hex_dump(const void *ptr, std::size_t size)
{
	const unsigned char	*memory;
	const char			digits[] = "0123456789ABCDEF";
	char				byte[3];
	std::size_t			i;

	memory = static_cast<const unsigned char *>(ptr);
	if (size > 32)
		size = 32;
	write_text("    data: ");
	i = 0;
	while (i < size)
	{
		byte[0] = digits[memory[i] / 16];
		byte[1] = digits[memory[i] % 16];
		byte[2] = ' ';
		write(STDOUT_FILENO, byte, sizeof(byte));
		i++;
	}
	write_text("\n");
}

static void	show_zone_details(t_zone_type type, const char *name)
{
	t_header_info	*zone;
	t_header_info	*header;
	std::uintptr_t	zone_address;

	zone = next_zone_by_address(type, 0);
	while (zone != nullptr)
	{
		zone_address = reinterpret_cast<std::uintptr_t>(zone);
		write_text("\n");
		write_text(name);
		write_text(" zone ");
		write_pointer(zone);
		write_text(" mapped=");
		write_number(zone->mapping_size, "0123456789");
		write_text(" bytes\n");
		header = zone;
		while (header != nullptr && header->zone == zone)
		{
			write_text("  ");
			if (header->is_free)
				write_text("FREE ");
			else
				write_text("USED ");
			write_pointer(header + 1);
			write_text(" requested=");
			write_number(header->requested_size, "0123456789");
			write_text(" capacity=");
			write_number(header->size, "0123456789");
			write_text("\n");
			if (!header->is_free)
				write_hex_dump(header + 1, header->requested_size);
			header = header->next;
		}
		zone = next_zone_by_address(type, zone_address);
	}
}

static void	show_history()
{
	const t_history_event	*event;
	std::size_t			count;
	std::size_t			i;

	count = get_history_count();
	i = 0;
	if (count > 32)
		i = count - 32;
	write_text("\nHistory (last ");
	write_number(count - i, "0123456789");
	write_text(" events)\n");
	while (i < count)
	{
		event = get_history_event(i);
		write_text("  #");
		write_number(event->id, "0123456789");
		write_text(" ");
		write_text(action_name(event->action));
		write_text(" ");
		if (event->old_ptr != nullptr && event->old_ptr != event->ptr)
		{
			write_pointer(event->old_ptr);
			write_text(" -> ");
		}
		write_pointer(event->ptr);
		write_text(" size=");
		write_number(event->size, "0123456789");
		write_text("\n");
		i++;
	}
}

static std::size_t	show_zone_type(t_zone_type type, const char *name)
{
	t_header_info	*zone;
	t_header_info	*header;
	std::uintptr_t	zone_address;
	std::uintptr_t	start;
	std::size_t	total;

	total = 0;
	zone = next_zone_by_address(type, 0);
	while (zone != nullptr)
	{
		zone_address = reinterpret_cast<std::uintptr_t>(zone);
		write_text(name);
		write_text(" : 0x");
		write_number(zone_address, "0123456789ABCDEF");
		write_text("\n");
		header = zone;
		while (header != nullptr && header->zone == zone)
		{
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

			header = header->next;
		}

		zone = next_zone_by_address(type, zone_address);
	}

	return (total);
}

extern "C" void	show_alloc_mem() noexcept
{
	std::size_t	total;

	lock_allocator();
	total = show_zone_type(TINY, "TINY");
	total += show_zone_type(SMALL, "SMALL");
	total += show_zone_type(LARGE, "LARGE");
	write_text("Total : ");
	write_number(total, "0123456789");
	write_text(" bytes\n");
	unlock_allocator();
}

extern "C" void	show_alloc_mem_ex() noexcept
{
	lock_allocator();
	initialize_debug_options();
	write_text("=== ft_malloc extended state ===\n");
	show_zone_details(TINY, "TINY");
	show_zone_details(SMALL, "SMALL");
	show_zone_details(LARGE, "LARGE");
	show_history();
	unlock_allocator();
}
