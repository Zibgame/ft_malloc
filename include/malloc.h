/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:15:39 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/25 13:25:36 by zcadinot         ###   ########.fr       */
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
