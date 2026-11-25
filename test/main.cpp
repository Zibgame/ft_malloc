/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 15:23:15 by zcadinot          #+#    #+#             */
/*   Updated: 2026/11/25 13:25:36 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc_internal.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

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
	std::size_t		size;
	std::size_t		i;

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
			if (ptr == nullptr || ptr[0] != value || ptr[size - 1] != value)
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

static int	fail(const char *name)
{
	std::fprintf(stderr, "ft_malloc: %s test failed\n", name);
	return (1);
}

int	main()
{
	void			*tiny;
	void			*small;
	void			*large;
	char			*text;
	char			*grown;
	void			*first;
	void			*second;
	void			*resized;
	void			*tiny_blocks[220];
	void			*small_blocks[110];
	pthread_t		threads[4];
	t_thread_test	thread_tests[4];
	std::size_t		impossible_size;
	std::size_t		i;
	long			page_size;

	free(nullptr);
	impossible_size = static_cast<std::size_t>(-1);
	if (malloc(0) != nullptr || malloc(impossible_size) != nullptr)
		return (fail("invalid size"));

	tiny = malloc(128);
	small = malloc(512);
	large = malloc(2048);
	if (tiny == nullptr || small == nullptr || large == nullptr)
		return (fail("allocation"));

	if (reinterpret_cast<std::uintptr_t>(tiny) % alignof(std::max_align_t) != 0
		|| reinterpret_cast<std::uintptr_t>(small) % alignof(std::max_align_t) != 0
		|| reinterpret_cast<std::uintptr_t>(large) % alignof(std::max_align_t) != 0)
		return (fail("alignment"));

	if (get_header(tiny)->type != TINY
		|| get_header(small)->type != SMALL
		|| get_header(large)->type != LARGE)
		return (fail("category"));

	page_size = sysconf(_SC_PAGESIZE);
	if (page_size <= 0
		|| get_header(tiny)->zone->mapping_size % page_size != 0
		|| get_header(small)->zone->mapping_size % page_size != 0
		|| get_header(large)->zone->mapping_size % page_size != 0)
		return (fail("page size"));

	free(tiny);
	free(small);
	free(large);

	text = static_cast<char *>(malloc(16));
	if (text == nullptr)
		return (fail("realloc setup"));

	std::strcpy(text, "ft_malloc");
	grown = static_cast<char *>(realloc(text, 4096));
	if (grown == nullptr || std::strcmp(grown, "ft_malloc") != 0)
		return (fail("realloc growth"));

	text = static_cast<char *>(realloc(grown, 16));
	if (text == nullptr || std::strcmp(text, "ft_malloc") != 0)
		return (fail("realloc shrink"));

	if (realloc(text, 0) != nullptr)
		return (fail("realloc zero"));

	text = static_cast<char *>(realloc(nullptr, 16));
	if (text == nullptr)
		return (fail("realloc null"));

	std::strcpy(text, "preserved");
	if (realloc(text, impossible_size) != nullptr
		|| std::strcmp(text, "preserved") != 0)
		return (fail("realloc failure"));

	free(text);

	first = malloc(64);
	second = malloc(64);
	if (first == nullptr || second == nullptr)
		return (fail("in-place setup"));

	std::memset(first, 42, 64);
	free(second);
	resized = realloc(first, 96);
	if (resized != first
		|| static_cast<unsigned char *>(resized)[0] != 42
		|| static_cast<unsigned char *>(resized)[63] != 42)
		return (fail("in-place realloc"));

	free(resized);

	i = 0;
	while (i < 220)
	{
		tiny_blocks[i] = malloc(128);
		if (tiny_blocks[i] == nullptr)
			return (fail("tiny stress"));

		i++;
	}

	i = 0;
	while (i < 110)
	{
		small_blocks[i] = malloc(1024);
		if (small_blocks[i] == nullptr)
			return (fail("small stress"));

		i++;
	}

	if (get_header(tiny_blocks[0])->zone == get_header(tiny_blocks[219])->zone
		|| get_header(small_blocks[0])->zone
			== get_header(small_blocks[109])->zone)
		return (fail("multiple zones"));

	while (i > 0)
		free(small_blocks[--i]);

	i = 220;
	while (i > 0)
		free(tiny_blocks[--i]);

	i = 0;
	while (i < 4)
	{
		thread_tests[i].id = i;
		thread_tests[i].success = false;
		if (pthread_create(&threads[i], nullptr, thread_worker,
				&thread_tests[i]) != 0)
			return (fail("thread creation"));

		i++;
	}

	i = 0;
	while (i < 4)
	{
		if (pthread_join(threads[i], nullptr) != 0
			|| !thread_tests[i].success)
			return (fail("thread stress"));

		i++;
	}

	tiny = malloc(32);
	small = malloc(512);
	large = malloc(2048);
	if (tiny == nullptr || small == nullptr || large == nullptr)
		return (fail("display setup"));

	show_alloc_mem();
	free(tiny);
	free(small);
	free(large);
	std::printf("ft_malloc tests: OK\n");
	return (0);
}
