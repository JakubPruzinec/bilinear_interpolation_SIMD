#include "main.h"
#include <immintrin.h>

int main(int argc, char *argv[])
{
    uint64_t    avgHash1 = 0x0000000000000000;
    uint64_t    avgHash2 = 0x0000000000000000;
    char        avgHashStr[HEX_PREFIXED_8B_STR_SIZE]; 
    image_t     *image1 = NULL;
    image_t     *image2 = NULL;

    RET_ERR_MSG(argc != 3, "./image-info <image1> <image2>\n");

    RET_ERR_MSG(!(image1 = imgLoadBitmap(argv[1])), "Failed to load a bitmap file, only"
                                                    " 24bpp BMS are supported so far\n");

    RET_ERR_MSG(!(image2 = imgLoadBitmap(argv[2])), "Failed to load a bitmap file, only"
                                                    " 24bpp BMS are supported so far\n");

    RET_ERR_MSG(!imgAvgHash(image1, &avgHash1), "Failed to compute average hash\n");
    RET_ERR_MSG(!imgAvgHash(image2, &avgHash2), "Failed to compute average hash\n");

    int64ToHexStr(avgHash1, avgHashStr);
    printf("%s average hash:\t%s\n", argv[1], avgHashStr);
    int64ToHexStr(avgHash2, avgHashStr);
    printf("%s average hash:\t%s\n", argv[2], avgHashStr);

    if (hemmingDistance(avgHash1, avgHash2) <= AVG_HASH_SIMILARITY_TRASHHOLD)
    {
        printf("[Images are SIMILAR]\n");
    }
    else
    {
        printf("[Images are DIFFERENT]\n");
    }

    // UNCOMMENT ME TO TEST RESIZING
    // image_t *resized = imgResize(image1, 800, 800);
    // if (resized)
    // {
    //  imgSaveBitmap(resized, "../test/resized_test.bmp");
    //  imgDestroy(resized);
    // }

    imgDestroy(image1);
    imgDestroy(image2);
    return 0;

error:
    if (image1) { imgDestroy(image1); }
    if (image2) { imgDestroy(image2); }
    return 1;
}