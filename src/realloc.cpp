/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:35:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/24 17:52:27 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

extern "C" void	*realloc(void *ptr, std::size_t size) noexcept
{
	void			*new_ptr;
	std::size_t	old_size;
	std::size_t	i;
	unsigned char	*source;
	unsigned char	*destination;

	if (ptr == nullptr)
		return (malloc(size));

	if (size == 0)
	{
		free(ptr);
		return (nullptr);
	}

	old_size = get_header(ptr)->requested_size;
	if (get_header_size(ptr) >= size)
	{
		get_header(ptr)->requested_size = size;
		return (ptr);
	}

	new_ptr = malloc(size);
	if (new_ptr == nullptr)
		return (nullptr);

	source = static_cast<unsigned char *>(ptr);
	destination = static_cast<unsigned char *>(new_ptr);
	i = 0;
	while (i < old_size)
	{
		destination[i] = source[i];
		i++;
	}
	free(ptr);
	return (new_ptr);
}
