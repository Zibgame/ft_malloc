/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 13:33:06 by zcadinot          #+#    #+#             */
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
	write(STDERR_FILENO, text, length);
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
	write(STDERR_FILENO, buffer + i, sizeof(buffer) - i);
}

static const char	*action_name(t_history_action action)
{
	if (action == HISTORY_MALLOC)
		return ("malloc");
	if (action == HISTORY_FREE)
		return ("free");
	return ("realloc");
}

static const char	*type_name(t_zone_type type)
{
	if (type == TINY)
		return ("TINY");
	if (type == SMALL)
		return ("SMALL");
	return ("LARGE");
}

void	write_debug_event(const t_history_event *event)
{
	write_text("[ft_malloc] #");
	write_number(event->id, "0123456789");
	write_text(" ");
	write_text(action_name(event->action));
	if (event->old_ptr != nullptr && event->old_ptr != event->ptr)
	{
		write_text(" 0x");
		write_number(reinterpret_cast<std::uintptr_t>(event->old_ptr),
			"0123456789ABCDEF");
		write_text(" ->");
	}
	write_text(" 0x");
	write_number(reinterpret_cast<std::uintptr_t>(event->ptr),
		"0123456789ABCDEF");
	write_text(" ");
	write_number(event->size, "0123456789");
	write_text(" bytes ");
	write_text(type_name(event->type));
	write_text("\n");
}
