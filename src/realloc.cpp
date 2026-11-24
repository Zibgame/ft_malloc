/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:35:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/24 15:57:23 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

extern "C" void	*realloc(void *ptr, std::size_t size) noexcept
{
	(void)ptr;
	(void)size;
	return (nullptr);
}
