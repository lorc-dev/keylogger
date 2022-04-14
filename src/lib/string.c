//
// Created by Lorenz on 13/04/2022.
//
#include <stdlib.h>

/**
 * strlen implementation
 *
 * @param str
 * @return
 */
size_t strlen(const char *str) {
    const char *s;
    for (s = str; *s; ++s);
    return (s - str);
}