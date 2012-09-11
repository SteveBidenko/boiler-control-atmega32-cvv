#include "dayofweek.h"

unsigned char dayofweek(unsigned char dd, unsigned char mm, unsigned char yy) {
    int yyyy, m, n;
    
    yyyy = 2000 + yy;
    m = (int)mm - 2;
    if (m <= 0) {
        m += 12;
        yyyy--;
    }
    n = ((int)dd + (13 * m - 1) / 5 + (yyyy % 100) + (yyyy / 100) / 4
        + (yyyy % 100) / 4 - 2 * (yyyy / 100)) % 7;
    if (n < 0) n += 7;
    
    return (n);
}