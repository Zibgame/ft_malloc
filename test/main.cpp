/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:23:15 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/24 15:23:39 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"
#include <cstdint>
#include <cstdio>
#include <cstring>

static bool	is_aligned(void *ptr)
{
	return (reinterpret_cast<std::uintptr_t>(ptr)
		% alignof(std::max_align_t) == 0);
}

static bool	test_categories()
{
	void	*tiny;
	void	*small;
	void	*large;
	bool	result;

	tiny = malloc(128);
	small = malloc(512);
	large = malloc(2048);
	result = tiny != nullptr && small != nullptr && large != nullptr;
	result = result && is_aligned(tiny) && is_aligned(small)
		&& is_aligned(large);
	result = result && get_header(tiny)->type == TINY;
	result = result && get_header(small)->type == SMALL;
	result = result && get_header(large)->type == LARGE;
	free(tiny);
	free(small);
	free(large);
	return (result);
}

static bool	test_realloc()
{
	char	*text;
	char	*grown;
	void	*from_null;
	std::size_t	impossible_size;

	text = static_cast<char *>(malloc(16));
	if (text == nullptr)
		return (false);

	std::strcpy(text, "ft_malloc");
	grown = static_cast<char *>(realloc(text, 4096));
	if (grown == nullptr || std::strcmp(grown, "ft_malloc") != 0)
		return (false);

	grown = static_cast<char *>(realloc(grown, 16));
	if (grown == nullptr || std::strcmp(grown, "ft_malloc") != 0)
		return (false);

	free(grown);
	from_null = realloc(nullptr, 64);
	if (from_null == nullptr)
		return (false);

	if (realloc(from_null, 0) != nullptr)
		return (false);

	text = static_cast<char *>(malloc(16));
	if (text == nullptr)
		return (false);

	std::strcpy(text, "preserved");
	impossible_size = static_cast<std::size_t>(-1);
	if (realloc(text, impossible_size) != nullptr
		|| std::strcmp(text, "preserved") != 0)
	{
		free(text);
		return (false);
	}

	free(text);
	return (true);
}

static bool	test_many_allocations()
{
	void	*tiny[120];
	void	*small[110];
	std::size_t	i;

	i = 0;
	while (i < 120)
	{
		tiny[i] = malloc(1 + i % 128);
		if (tiny[i] == nullptr)
			return (false);

		i++;
	}

	i = 0;
	while (i < 110)
	{
		small[i] = malloc(129 + i % 896);
		if (small[i] == nullptr)
			return (false);

		i++;
	}

	while (i > 0)
		free(small[--i]);

	i = 120;
	while (i > 0)
		free(tiny[--i]);

	return (true);
}

int	main()
{
	void	*tiny;
	void	*small;
	void	*large;
	std::size_t	impossible_size;

	free(nullptr);
	impossible_size = static_cast<std::size_t>(-1);
	if (malloc(0) != nullptr || malloc(impossible_size) != nullptr
		|| !test_categories()
		|| !test_realloc() || !test_many_allocations())
	{
		std::fprintf(stderr, "ft_malloc tests: FAILED\n");
		return (1);
	}

	tiny = malloc(32);
	small = malloc(512);
	large = malloc(2048);
	if (tiny == nullptr || small == nullptr || large == nullptr)
		return (1);

	show_alloc_mem();
	free(tiny);
	free(small);
	free(large);
	std::printf("ft_malloc tests: OK\n");

	return (0);
}
