#include "image.h"
#include "avx_general.h"

/**
 * Reads interpolated channel value
 * This function substitues the scalar alternative:
 * float redNew = imgReadChannel(rChannel, width, r, c) * (1.0 - deltaR) * (1.0 - deltaC)
 *                          + imgReadChannel(rChannel, width, r + 1, c) * deltaR * (1.0 - deltaC)
 *                          + imgReadChannel(rChannel, width, r, c + 1) * (1.0 - deltaR) * deltaC
 *                          + imgReadChannel(rChannel, width, r + 1, c + 1) * deltaR * deltaC;
 * @param channel Channel to read from
 * @param width Width of image
 * @param r Row (indexed from 0)
 * @param c_int_vec Vector of columns (indexed from 0)
 * @param delta_r_flt_vec Vector of deltaR
 * @param delta_c_flt_vec Vector of deltaC
 * @param one_minus_delta_r_flt_vec Vector of (1.0 - deltaR)
 * @param one_minus_delta_c_flt_vec Vector of (1.0 - deltaC)
 * @return Interpolated channel value vector
 */
static inline __m256 readNewChannel(const uint8_t *channel, size_t width, size_t r, __m256i c_int_vec,
                                    __m256 w1_vec, __m256 w2_vec, __m256 w3_vec, __m256 w4_vec)
{
    float values[AVX_REG_N_FLOATS] __attribute__((aligned(32)));
    __m256 tmp_flt_vec;
    __m256 new_val_flt_vec;

    /* new_val_flt_vec = imgReadChannel(channel, width, r, c) */
    avxImgReadChannelVec(channel, width, r, c_int_vec, 0, values);
    new_val_flt_vec = _mm256_load_ps(&values[0]);

    /* new_val_flt_vec *= (1.0 - deltaR) * (1.0 - deltaC) */
    new_val_flt_vec = _mm256_mul_ps(new_val_flt_vec, w1_vec);



    /* tmp = imgReadChannel(channel, width, r + 1, c) */
    avxImgReadChannelVec(channel, width, r + 1, c_int_vec, 0, values);
    tmp_flt_vec = _mm256_load_ps(&values[0]);

    /* tmp *= deltaR * (1.0 - deltaC) */
    tmp_flt_vec = _mm256_mul_ps(tmp_flt_vec, w2_vec);

    /* new_val_flt_vec += tmp_flt_vec */
    new_val_flt_vec = _mm256_add_ps(new_val_flt_vec, tmp_flt_vec);



    /* tmp = imgReadChannel(channel, width, r, c + 1) */
    avxImgReadChannelVec(channel, width, r, c_int_vec, 1, values);
    tmp_flt_vec = _mm256_load_ps(&values[0]);

    /* tmp *= (1.0 - deltaR) * deltaC */
    tmp_flt_vec = _mm256_mul_ps(tmp_flt_vec, w3_vec);

    /* new_val_flt_vec += tmp_flt_vec */
    new_val_flt_vec = _mm256_add_ps(new_val_flt_vec, tmp_flt_vec);



    /* tmp = imgReadChannel(channel, width, r + 1, c + 1) */
    avxImgReadChannelVec(channel, width, r + 1, c_int_vec, 1, values);
    tmp_flt_vec = _mm256_load_ps(&values[0]);

    /* tmp *= deltaR * deltaC */
    tmp_flt_vec = _mm256_mul_ps(tmp_flt_vec, w4_vec);

    /* new_val_flt_vec += tmp_flt_vec */
    new_val_flt_vec = _mm256_add_ps(new_val_flt_vec, tmp_flt_vec);  

    return new_val_flt_vec;
}

/*
** Necessary extensions:
**      AVX
** Suggested additional extensions:
**      AVX512
**          - process 16 pixels instead of 8
*/

image_t *imgResize(const image_t *img, size_t newWidth, size_t newHeight)
{
    float           sr = 0.0;                                       // row scale
    float           sc __attribute__((aligned (32))) = 0.0;         // column scale
    uint32_t        height = 0;
    uint32_t        width = 0;
    const uint8_t   *rChannel = NULL;
    const uint8_t   *gChannel = NULL;
    const uint8_t   *bChannel = NULL;
    image_t         *newImg = NULL;
    uint8_t         *newRChannel = NULL;
    uint8_t         *newGChannel = NULL;
    uint8_t         *newBChannel = NULL;

    __m256 index_vec = _mm256_set_ps(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    __m256 eight_flt_vec = _mm256_set_ps(8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0);
    __m256 one_flt_vec = _mm256_set_ps(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);

    RET_ERR_MSG(!img, "NULL image\n");
    RET_ERR_MSG(newWidth <= 1 || newHeight <= 1, "Invalid dimension\n");

    RET_ERR_MSG(!(newImg = imgCreate(newWidth, newHeight)), "Allocation error\n");

    height = (uint32_t)img->height;
    width = (uint32_t)img->width;
    rChannel = img->rChannel;
    gChannel = img->gChannel;
    bChannel = img->bChannel;
    newRChannel = newImg->rChannel;
    newGChannel = newImg->gChannel;
    newBChannel = newImg->bChannel;
    sr = (float)height / (float)newHeight;
    sc = (float)width / (float)newWidth;

    /* max_width = width - 2 */
    float max_width __attribute__((aligned (32))) = width - 2.0;
    __m256 max_width_vec = _mm256_broadcast_ss(&max_width);

    float rf = 0.0;
    for (size_t rNew = 0; rNew < newHeight; rNew++, rf += sr)
    {
        size_t r = (size_t)rf;
        r = (r > height - 2) ? height - 2 : r;
        const float deltaR __attribute__((aligned (32))) = rf - r;
        const float oneMinusDeltaR = 1.0 - deltaR;

        /* deltaR = rf - r */
        __m256 delta_r_flt_vec = _mm256_broadcast_ss(&deltaR);
        /* (1.0 - deltaR) */
        __m256 one_minus_delta_r_flt_vec = _mm256_broadcast_ss(&oneMinusDeltaR);

        /* cf_flt_vec = [0sc | 1sc | 2sc | 3sc | 4sc | ...] */
        __m256 sc_flt_vec = _mm256_broadcast_ss(&sc);
        __m256 cf_flt_vec = _mm256_mul_ps(index_vec, sc_flt_vec);
        __m256 eight_sc_flt_vec = _mm256_mul_ps(eight_flt_vec, sc_flt_vec);
        /* process AVX_REG_N_FLOATS pixels in one iteration */
        size_t cNew;
        for (cNew = 0; cNew + AVX_REG_N_FLOATS < newWidth; cNew += AVX_REG_N_FLOATS)
        {
            __m256 new_val_flt_vec;

            /* c = (uint32_t)cf */
            __m256 c_flt_vec = _mm256_floor_ps(cf_flt_vec);
            /* c = (c <= maxWidth) ? c : maxWidth */
            __m256 mask_vec = _mm256_cmp_ps(c_flt_vec, max_width_vec, _CMP_LE_OS);
            c_flt_vec = _mm256_and_ps(c_flt_vec, mask_vec);
            mask_vec = avx_mm256_not_ps(mask_vec);
            c_flt_vec = _mm256_or_ps(c_flt_vec, _mm256_and_ps(max_width_vec, mask_vec));
            __m256i c_int_vec = _mm256_cvttps_epi32(c_flt_vec);

            /* deltaC = cf - c */
            __m256 delta_c_flt_vec = _mm256_sub_ps(cf_flt_vec, c_flt_vec);
            /* (1.0 - deltaC) */
            __m256 one_minus_delta_c_flt_vec = _mm256_sub_ps(one_flt_vec, delta_c_flt_vec);

            /*
                w1 = (1.0 - deltaR) * (1.0 - deltaC);
                w2 = deltaR * (1.0 - deltaC);
                w3 = (1.0 - deltaR) * deltaC;
                w4 = deltaR * deltaC;
            */
            __m256 w1_vec = _mm256_mul_ps(one_minus_delta_r_flt_vec, one_minus_delta_c_flt_vec);
            __m256 w2_vec = _mm256_mul_ps(delta_r_flt_vec, one_minus_delta_c_flt_vec);
            __m256 w3_vec = _mm256_mul_ps(one_minus_delta_r_flt_vec, delta_c_flt_vec);
            __m256 w4_vec = _mm256_mul_ps(delta_r_flt_vec, delta_c_flt_vec);



            /* new_val_flt_vec = imgReadChannel(...) * ... * + imgReadChannel(...) * ... * ... */
            new_val_flt_vec = readNewChannel(rChannel, width, r, c_int_vec,
                                            w1_vec, w2_vec, w3_vec, w4_vec);

            /* imgWriteChannel(newRChannel, newWidth, rNew, cNew, redNew) */
            avxImgWriteChannelVec(newRChannel, newWidth, rNew, cNew, new_val_flt_vec);

            /* new_val_flt_vec = imgReadChannel(...) * ... * + imgReadChannel(...) * ... * ... */
            new_val_flt_vec = readNewChannel(gChannel, width, r, c_int_vec,
                                            w1_vec, w2_vec, w3_vec, w4_vec);

            /* imgWriteChannel(newGChannel, newWidth, rNew, cNew, redNew) */
            avxImgWriteChannelVec(newGChannel, newWidth, rNew, cNew, new_val_flt_vec);

            /* new_val_flt_vec = imgReadChannel(...) * ... * + imgReadChannel(...) * ... * ... */
            new_val_flt_vec = readNewChannel(bChannel, width, r, c_int_vec,
                                            w1_vec, w2_vec, w3_vec, w4_vec);

            /* imgWriteChannel(newBChannel, newWidth, rNew, cNew, redNew) */
            avxImgWriteChannelVec(newBChannel, newWidth, rNew, cNew, new_val_flt_vec);



            /* cf = (8 * sc) + cf */
            cf_flt_vec = _mm256_add_ps(eight_sc_flt_vec, cf_flt_vec);
        }

        /* finished the rest */
        float cf = cNew * sc;
        for ( ; cNew < newWidth; cNew++, cf += sc)
        {
            size_t c = (size_t)cf;
            c = (c > width - 2) ? width - 2 : c;
            float deltaC = cf - c;

            float redNew = imgReadChannel(rChannel, width, r, c) * (1.0 - deltaR) * (1.0 - deltaC)
                            + imgReadChannel(rChannel, width, r + 1, c) * deltaR * (1.0 - deltaC)
                            + imgReadChannel(rChannel, width, r, c + 1) * (1.0 - deltaR) * deltaC
                            + imgReadChannel(rChannel, width, r + 1, c + 1) * deltaR * deltaC;

            float greenNew = imgReadChannel(gChannel, width, r, c) * (1.0 - deltaR) * (1.0 - deltaC)
                            + imgReadChannel(gChannel, width, r + 1, c) * deltaR * (1.0 - deltaC)
                            + imgReadChannel(gChannel, width, r, c + 1) * (1.0 - deltaR) * deltaC
                            + imgReadChannel(gChannel, width, r + 1, c + 1) * deltaR * deltaC;

            float blueNew = imgReadChannel(bChannel, width, r, c) * (1.0 - deltaR) * (1.0 - deltaC)
                            + imgReadChannel(bChannel, width, r + 1, c) * deltaR * (1.0 - deltaC)
                            + imgReadChannel(bChannel, width, r, c + 1) * (1.0 - deltaR) * deltaC
                            + imgReadChannel(bChannel, width, r + 1, c + 1) * deltaR * deltaC;

            imgWriteChannel(newRChannel, newWidth, rNew, cNew, redNew);
            imgWriteChannel(newGChannel, newWidth, rNew, cNew, greenNew);
            imgWriteChannel(newBChannel, newWidth, rNew, cNew, blueNew);
        }
    }

    return newImg;

error:
    if (newImg) { imgDestroy(newImg); }
    return NULL;
}