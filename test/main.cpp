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
#include <pthread.h>
#include <unistd.h>

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
	long	page_size;

	tiny = malloc(128);
	small = malloc(512);
	large = malloc(2048);
	result = tiny != nullptr && small != nullptr && large != nullptr;
	result = result && is_aligned(tiny) && is_aligned(small)
		&& is_aligned(large);
	result = result && get_header(tiny)->type == TINY;
	result = result && get_header(small)->type == SMALL;
	result = result && get_header(large)->type == LARGE;
	page_size = sysconf(_SC_PAGESIZE);
	result = result && page_size > 0;
	result = result && get_header(tiny)->zone->mapping_size % page_size == 0;
	result = result && get_header(small)->zone->mapping_size % page_size == 0;
	result = result && get_header(large)->zone->mapping_size % page_size == 0;
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

static bool	test_realloc_in_place()
{
	void	*first;
	void	*second;
	void	*resized;

	first = malloc(64);
	second = malloc(64);
	if (first == nullptr || second == nullptr)
		return (false);

	std::memset(first, 42, 64);
	free(second);
	resized = realloc(first, 96);
	if (resized != first)
	{
		free(resized);
		return (false);
	}

	if (static_cast<unsigned char *>(resized)[0] != 42
		|| static_cast<unsigned char *>(resized)[63] != 42)
	{
		free(resized);
		return (false);
	}

	free(resized);
	return (true);
}

static bool	test_many_allocations()
{
	void	*tiny[220];
	void	*small[110];
	std::size_t	i;

	i = 0;
	while (i < 220)
	{
		tiny[i] = malloc(128);
		if (tiny[i] == nullptr)
			return (false);

		i++;
	}

	i = 0;
	while (i < 110)
	{
		small[i] = malloc(1024);
		if (small[i] == nullptr)
			return (false);

		i++;
	}
	if (get_header(tiny[0])->zone == get_header(tiny[219])->zone
		|| get_header(small[0])->zone == get_header(small[109])->zone)
		return (false);

	while (i > 0)
		free(small[--i]);

	i = 220;
	while (i > 0)
		free(tiny[--i]);

	return (true);
}

typedef struct s_thread_test
{
	std::size_t	id;
	bool			success;
} t_thread_test;

static void	*thread_worker(void *argument)
{
	t_thread_test	*test;
	unsigned char	*ptr;
	unsigned char	value;
	std::size_t	size;
	std::size_t	i;

	test = static_cast<t_thread_test *>(argument);
	test->success = true;
	i = 0;
	while (i < 2000)
	{
		size = 1 + (i * 37 + test->id * 101) % 4096;
		value = static_cast<unsigned char>(i + test->id);
		ptr = static_cast<unsigned char *>(malloc(size));
		if (ptr == nullptr)
		{
			test->success = false;
			return (nullptr);
		}

		ptr[0] = value;
		ptr[size - 1] = value;
		if (i % 3 == 0)
		{
			ptr = static_cast<unsigned char *>(realloc(ptr, size + 200));
			if (ptr == nullptr || ptr[0] != value
				|| ptr[size - 1] != value)
			{
				test->success = false;
				return (nullptr);
			}
		}

		free(ptr);
		i++;
	}

	return (nullptr);
}

static bool	test_threads()
{
	pthread_t		threads[4];
	t_thread_test	tests[4];
	std::size_t		i;

	i = 0;
	while (i < 4)
	{
		tests[i].id = i;
		tests[i].success = false;
		if (pthread_create(&threads[i], nullptr, thread_worker, &tests[i]) != 0)
			return (false);

		i++;
	}

	i = 0;
	while (i < 4)
	{
		if (pthread_join(threads[i], nullptr) != 0 || !tests[i].success)
			return (false);

		i++;
	}

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
		|| !test_realloc() || !test_realloc_in_place()
		|| !test_many_allocations() || !test_threads())
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
