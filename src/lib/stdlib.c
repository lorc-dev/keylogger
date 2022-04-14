//
// Created by Lorenz on 12/04/2022.
//

#include <stdlib.h>

static void str_reverse(char *str, int len) {
    int start = 0;
    int end = len - 1;
    char temp;
    while (start < end)
    {
        temp = *(str+start);
        *(str+start) = *(str+end);
        *(str+end) = temp;
        start++;
        end--;
    }
}

/**
 * itoa implementation.
 * Modified, original from: https://www.codevscolor.com/c-itoa-function
 *
 * @param num
 * @param buffer
 * @param base
 * @return
 */
char * itoa(int num, char *buffer, int base) {
    int i = 0, rem, negative = 0;

    if (num == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return buffer;
    }

    if (num < 0 && base == 10) {
        num *= -1;
        negative = 1;
    }

    while (num != 0) {
        rem = num % base;
        buffer[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num /= base;
    }

    if (negative) {
        buffer[i++] = '-';
    }

    str_reverse(buffer, i);

    buffer[i] = '\0';

    return buffer;
}