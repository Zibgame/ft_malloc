/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:35:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/24 16:22:48 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"
#include <sys/mman.h>

extern "C" void	free(void *ptr) noexcept
{
	if (ptr == nullptr)
		return ;
	munmap(get_header_pos(ptr), sizeof(t_header_info) + get_header_size(ptr));
}
