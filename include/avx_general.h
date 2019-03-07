#ifndef _AVX_GENERAL_H_
#define _AVX_GENERAL_H_

#include <immintrin.h>
#include <stdint.h>

#define AVX_REG_N_FLOATS        8

/**
 * Dumps a __m256 register to stderr
 * @param x Register to dump
 */
#define avx_dump_sp256(x)   do {                                                    \
    float __avx_dump_sp256_var_tmp[AVX_REG_N_FLOATS] __attribute__((aligned (32))); \
    _mm256_store_ps(&__avx_dump_sp256_var_tmp[0], (x));                             \
    fprintf(stderr, "MSB ");                                                        \
    for (size_t i = 0; i < AVX_REG_N_FLOATS; i++)                                   \
    {                                                                               \
        fprintf(stderr, "%.2f", __avx_dump_sp256_var_tmp[i]);                       \
        if (i < AVX_REG_N_FLOATS - 1)                                               \
        {                                                                           \
            fprintf(stderr, " | ");                                                 \
        }                                                                           \
    }                                                                               \
    fprintf(stderr, " LSB\n");                                                      \
} while (0)

/**
 * Dumps a __m256i register to stderr
 * @param x Register to dump
 */
#define avx_dump_si256(x)   do {                                                        \
    uint32_t __avx_dump_si256_var_tmp[AVX_REG_N_FLOATS] __attribute__((aligned (32)));  \
    _mm256_store_si256((__m256i *)&__avx_dump_si256_var_tmp[0], (x));                   \
    fprintf(stderr, "MSB ");                                                            \
    for (size_t i = 0; i < AVX_REG_N_FLOATS; i++)                                       \
    {                                                                                   \
        fprintf(stderr, "%d", __avx_dump_si256_var_tmp[i]);                             \
        if (i < AVX_REG_N_FLOATS - 1)                                                   \
        {                                                                               \
            fprintf(stderr, " | ");                                                     \
        }                                                                               \
    }                                                                                   \
    fprintf(stderr, " LSB\n");                                                          \
} while (0)

/**
 * Performs a logical negation
 * @param v Value to negate
 * @return Negated value
 */
static inline __m256 avx_mm256_not_ps(const __m256 v)
{
    __m256i vi = _mm256_castps_si256(v);
    vi = ~vi;
    return _mm256_castsi256_ps(vi);
}

/**
 * Read channel value at given possition. Bounds are not checked
 * @param channel Pointer to channel array
 * @param width Image width
 * @param r Row (indexed from 0)
 * @param cVec Column vector (indexed from 0), __m256i
 * @param cVecOffset Column vector offset, e.g cVec = [1,2,3, ...] cVecOffset = 3 --> [4,5,6, ...]
 * @param resVec Float array of at least AVX_REG_N_FLOATS size aligned to 32b to store the result to
 */
#define avxImgReadChannelVec(channel, width, r, cVec, cVecOffset, resVec)                   \
do {                                                                                        \
    uint32_t __avxImgReadChannelVarTmp[AVX_REG_N_FLOATS] __attribute__((aligned (32)));     \
    _mm256_store_si256((__m256i *)__avxImgReadChannelVarTmp, (cVec));                       \
    size_t __avxImgReadChannelVarTmpOffset = (r) * (width) + (cVecOffset);                  \
    resVec[0] = (channel)[__avxImgReadChannelVarTmpOffset + __avxImgReadChannelVarTmp[0]];  \
    resVec[1] = (channel)[__avxImgReadChannelVarTmpOffset + __avxImgReadChannelVarTmp[1]];  \
    resVec[2] = (channel)[__avxImgReadChannelVarTmpOffset + __avxImgReadChannelVarTmp[2]];  \
    resVec[3] = (channel)[__avxImgReadChannelVarTmpOffset + __avxImgReadChannelVarTmp[3]];  \
    resVec[4] = (channel)[__avxImgReadChannelVarTmpOffset + __avxImgReadChannelVarTmp[4]];  \
    resVec[5] = (channel)[__avxImgReadChannelVarTmpOffset + __avxImgReadChannelVarTmp[5]];  \
    resVec[6] = (channel)[__avxImgReadChannelVarTmpOffset + __avxImgReadChannelVarTmp[6]];  \
    resVec[7] = (channel)[__avxImgReadChannelVarTmpOffset + __avxImgReadChannelVarTmp[7]];  \
} while (0)

/**
 * Write channel value at given possition. Bounds are not checked
 * @param channel Pointer to channel array
 * @param width Image width
 * @param r Row (indexed from 0)
 * @param c Column (indexed from 0)
 * @param valVec Vector of values to write, __m256
 */
#define avxImgWriteChannelVec(channel, width, r, c, valVec)                                                 \
do {                                                                                                        \
    uint32_t __avxImgWriteChannelVecTmp[AVX_REG_N_FLOATS] __attribute__((aligned (32)));                    \
    _mm256_store_si256((__m256i *)__avxImgWriteChannelVecTmp, _mm256_cvttps_epi32(valVec));                 \
    size_t __avxImgWriteChannelVecTmpOffset = (r) * (width) + (c);                                          \
    ((uint8_t *)(channel))[__avxImgWriteChannelVecTmpOffset + 0] = (uint8_t)__avxImgWriteChannelVecTmp[7];  \
    ((uint8_t *)(channel))[__avxImgWriteChannelVecTmpOffset + 1] = (uint8_t)__avxImgWriteChannelVecTmp[6];  \
    ((uint8_t *)(channel))[__avxImgWriteChannelVecTmpOffset + 2] = (uint8_t)__avxImgWriteChannelVecTmp[5];  \
    ((uint8_t *)(channel))[__avxImgWriteChannelVecTmpOffset + 3] = (uint8_t)__avxImgWriteChannelVecTmp[4];  \
    ((uint8_t *)(channel))[__avxImgWriteChannelVecTmpOffset + 4] = (uint8_t)__avxImgWriteChannelVecTmp[3];  \
    ((uint8_t *)(channel))[__avxImgWriteChannelVecTmpOffset + 5] = (uint8_t)__avxImgWriteChannelVecTmp[2];  \
    ((uint8_t *)(channel))[__avxImgWriteChannelVecTmpOffset + 6] = (uint8_t)__avxImgWriteChannelVecTmp[1];  \
    ((uint8_t *)(channel))[__avxImgWriteChannelVecTmpOffset + 7] = (uint8_t)__avxImgWriteChannelVecTmp[0];  \
} while (0)


#endif