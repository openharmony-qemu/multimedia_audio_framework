/*
 * Stub header for x86 host build.
 * Maps securec functions to standard C library equivalents.
 */
#ifndef SECUREC_H
#define SECUREC_H

#include <cstring>
#include <cstddef>

inline int memcpy_s(void *dest, size_t destSize, const void *src, size_t count)
{
    if (dest == nullptr || src == nullptr || destSize < count) {
        return -1;
    }
    memcpy(dest, src, count);
    return 0;
}

inline int memset_s(void *dest, size_t destSize, int ch, size_t count)
{
    if (dest == nullptr || destSize < count) {
        return -1;
    }
    memset(dest, ch, count);
    return 0;
}

#endif  // SECUREC_H
