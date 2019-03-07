#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdbool.h>
#include <stdint.h>

#include "image.h"


#define HEX_PREFIXED_8B_STR_SIZE    11

#define RET_ERR(x)          do {if ((x)) {goto error;}} while (0)
#define RET_ERR_MSG(x, m)   do {if ((x)) {fprintf(stderr, (m)); goto error;}} while (0)

static inline uint16_t swap16(uint16_t v)
{
    return v << 8 | v >> 8;
}

static inline int32_t swap32(uint32_t v)
{
    return v << 24 | (v & 0x0000FF00) << 8 | (v & 0x00FF0000) >> 8 | v >> 24;
}

static inline void int64ToHexStr(uint64_t value, char res[HEX_PREFIXED_8B_STR_SIZE])
{
    if (!res) { return; }
    snprintf(res, HEX_PREFIXED_8B_STR_SIZE, "0x%08zx", value);
}

static inline size_t hemmingDistance(uint64_t v1, uint64_t v2)
{
    size_t distance = 0;
    for (size_t i = 0; i < 64; i++)
    {
        if (((v1 >> i) & 0x1) != ((v2 >> i) & 0x1))
        {
            distance++;
        }
    }
    return distance;
}

#endif // guardian