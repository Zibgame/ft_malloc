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

#include <cstdlib>
#include <cstdio>
#include <cstring>

int main()
{
    char* ptr = static_cast<char*>(malloc(32));

    if (!ptr)
        return 1;

    strcpy(ptr, "Hello World!");

    printf("Address: %p\n", static_cast<void*>(ptr));
    printf("Content: %s\n", ptr);

    free(ptr);
    return 0;
}
