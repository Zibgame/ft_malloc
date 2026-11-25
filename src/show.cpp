/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   show.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 13:13:30 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/25 13:25:36 by zcadinot         ###   ########.fr       */
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
