//
// Created by Lorenz on 21/02/2022.
// From https://github.com/gcc-mirror/gcc/blob/master/libgcc
// Public domain
//
#include <stdint.h>
#include <stddef.h>

extern void* memset (void *dest, int val, size_t len)
{
    unsigned char *ptr = dest;
    while (len-- > 0)
        *ptr++ = val;
    return dest;
}

extern int memcmp (const void *str1, const void *str2, size_t count)
{
    const unsigned char *s1 = str1;
    const unsigned char *s2 = str2;

    while (count-- > 0)
    {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }
    return 0;
}

extern void * memcpy (void *dest, const void *src, size_t len)
{
    char *d = dest;
    const char *s = src;
    while (len--)
        *d++ = *s++;
    return dest;
}

extern void * memmove (void *dest, const void *src, size_t len)
{
    char *d = dest;
    const char *s = src;
    if (d < s)
        while (len--)
            *d++ = *s++;
    else
    {
        char *lasts = s + (len-1);
        char *lastd = d + (len-1);
        while (len--)
            *lastd-- = *lasts--;
    }
    return dest;
}