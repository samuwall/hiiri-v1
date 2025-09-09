/**********************************************************************************
 ** file            : utils.c
 ** description     : simple implementations of libc functions
 ** 
 **********************************************************************************/

#include <stdint.h>
#include <stddef.h>

/* simple byte-loop 
 */
void *my_memcpy(void *dest, const void *src, size_t len) {

    uint8_t *d = dest;
    const uint8_t *s = src;

    while(len--) {
        *d++ = *s++;
    }
    return dest;
}

void *my_memset(void *dest, int c, size_t len) {
    
    uint8_t *p = dest;
    while (len--) {
        *p++ = (uint8_t)c;
    }
    return dest;
}

size_t my_strlen(const char *str) {

    const char *s = str;
    while (*s) {
        s++;
    }
    return s - str;
}