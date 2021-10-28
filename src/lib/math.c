//
// Created by Lorenz on 26/10/2021.
//

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