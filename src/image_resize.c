#include "image.h"

image_t *imgResize(const image_t *img, size_t newWidth, size_t newHeight)
{
    float           sr = 0.0;               // row scale
    float           sc = 0.0;               // column scale
    size_t          height = 0;
    size_t          width = 0;
    const uint8_t   *rChannel = NULL;
    const uint8_t   *gChannel = NULL;
    const uint8_t   *bChannel = NULL;
    image_t         *newImg = NULL;
    uint8_t         *newRChannel = NULL;
    uint8_t         *newGChannel = NULL;
    uint8_t         *newBChannel = NULL;

    RET_ERR_MSG(!img, "NULL image\n");
    RET_ERR_MSG(newWidth <= 1 || newHeight <= 1, "Invalid dimension\n");

    RET_ERR_MSG(!(newImg = imgCreate(newWidth, newHeight)), "Allocation error\n");

    height = img->height;
    width = img->width;
    rChannel = img->rChannel;
    gChannel = img->gChannel;
    bChannel = img->bChannel;
    newRChannel = newImg->rChannel;
    newGChannel = newImg->gChannel;
    newBChannel = newImg->bChannel;
    sr = (float)height / (float)newHeight;
    sc = (float)width / (float)newWidth;


    float rf = 0.0;
    for (size_t rNew = 0; rNew < newHeight; rNew++, rf += sr)
    {
        size_t r = (size_t)rf;
        r = (r > height - 2) ? height - 2 : r;
        float deltaR = rf - r;
        float oneMinusDeltaR = 1.0 - deltaR;

        float cf = 0.0;
        for (size_t cNew = 0; cNew < newWidth; cNew++, cf += sc)
        {
            size_t c = (size_t)cf;
            c = (c > width - 2) ? width - 2 : c;
            float deltaC = cf - c;
            float w1 = oneMinusDeltaR * (1.0 - deltaC);
            float w2 = deltaR * (1.0 - deltaC);
            float w3 = oneMinusDeltaR * deltaC;
            float w4 = deltaR * deltaC;

            float redNew = imgReadChannel(rChannel, width, r, c) * w1
                            + imgReadChannel(rChannel, width, r + 1, c) * w2
                            + imgReadChannel(rChannel, width, r, c + 1) * w3
                            + imgReadChannel(rChannel, width, r + 1, c + 1) * w4;

            float greenNew = imgReadChannel(gChannel, width, r, c) * w1
                            + imgReadChannel(gChannel, width, r + 1, c) * w2
                            + imgReadChannel(gChannel, width, r, c + 1) * w3
                            + imgReadChannel(gChannel, width, r + 1, c + 1) * w4;

            float blueNew = imgReadChannel(bChannel, width, r, c) * w1
                            + imgReadChannel(bChannel, width, r + 1, c) * w2
                            + imgReadChannel(bChannel, width, r, c + 1) * w3
                            + imgReadChannel(bChannel, width, r + 1, c + 1) * w4;

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