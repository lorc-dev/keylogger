//
// Created by Lorenz on 26/10/2021.
//

#include <stddef.h>
#include <stdint.h>
#include "../include/lib/math.h"

/**
 * Slow integer division (floor)
 * @note 0/0 will return 0
 *
 * @param dividend
 * @param divisor
 * @return
 */
uint32_t integer_division(uint32_t dividend, uint32_t divisor) {
    uint32_t result = 0;

    // Can't divide by zero
    if (divisor == 0)
        return 0;

    while (dividend >= divisor) {
        dividend -= divisor;
        result++;
    }

    return result;
}

/**
 * Slow integer division (ceil)
 * @note 0/0 will return 0
 *
 * @param dividend
 * @param divisor
 * @return
 */
uint32_t ceil_division(uint32_t dividend, uint32_t divisor) {
    // Can't divide by zero
    if (divisor == 0)
        return 0;

    uint32_t product = 0;
    while (dividend >= divisor) {
        dividend -= divisor;
        ++product;
    }
    if (dividend != 0)
        ++product;

    return product;
}

size_t min(size_t p1, size_t p2) {
    if (p1 < p2)
        return p1;
    return p2;
}

size_t max(size_t p1, size_t p2) {
    if (p1 > p2)
        return p1;
    return p2;
}