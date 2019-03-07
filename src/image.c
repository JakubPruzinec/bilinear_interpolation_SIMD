#include <endian.h>
#include "image.h"

image_t *imgLoadBitmap(const char *bmpFile)
{
    FILE        *f = NULL;
    image_t     *img = NULL;
    uint8_t     *pixelArray = NULL;
    size_t      pixelArrayLen = 0;
    size_t      nPixels = 0;
    size_t      nPadding = 0;
    size_t      nRead = 0;
    bmp_hdr_t   bmpHdr;                     // used as dummy here
    dib_hdr_t   dibHdr;
    size_t      width = 0;
    size_t      height = 0;
    uint8_t     *rChannel = NULL;
    uint8_t     *gChannel = NULL;
    uint8_t     *bChannel = NULL;

    RET_ERR_MSG(!bmpFile, "NULL file name\n");
    RET_ERR_MSG(!(f = fopen(bmpFile, "rb")), "Failed to open file\n");
    RET_ERR_MSG((nRead = fread(&bmpHdr, sizeof(bmpHdr), 1, f)) != 1, "Reading error\n");

#if __BYTE_ORDER == __BIG_ENDIAN
    bmpHdr.magicNumber = swap16(magicNumber);
    bmpHdr.fileSize = swap32(fileSize);
    bmpHdr.reserved1 = swap16(reserved1);
    bmpHdr.reserved2 = swap16(reserved2);
    bmpHdr.pixelsOffset = swap32(pixelsOffset);
#endif

    // dumpBmpHeader(&bmpHdr);
    RET_ERR_MSG(bmpHdr.magicNumber != BMP_MAGIC_NUMBER, "File is not a bitmap\n");
    RET_ERR_MSG((nRead = fread(&dibHdr, sizeof(dibHdr), 1, f)) != 1, "Reading error\n");

#if __BYTE_ORDER == __BIG_ENDIAN
    dibHdr.hdrSize = swap32(hdrSize);
    dibHdr.width = swap32(width);
    dibHdr.height = swap32(height);
    dibHdr.cPlanes = swap16(cPlanes);
    dibHdr.bpp = swap16(bpp);
    dibHdr.compression = swap32(compression);
    dibHdr.imgSize = swap32(imgSize);
    dibHdr.hResolution = swap32(hResolution);
    dibHdr.vResolution = swap32(vResolution);
    dibHdr.nColors = swap32(nColors);
    dibHdr.nImpColors = swap32(nImpColors);
#endif

    // dumpDibHeader(&dibHdr);

    RET_ERR_MSG(dibHdr.cPlanes != 1, "Corrupted DIB color planes\n");
    RET_ERR_MSG(dibHdr.compression != 0, "DIB compression is unsupported\n");
    RET_ERR_MSG(dibHdr.bpp != 24, "DIB bpp other than 24bpp is unsupported\n");
    RET_ERR_MSG(dibHdr.width < 0 || dibHdr.height < 0, "Negative DIB dimenstions unsupported\n");

    RET_ERR_MSG(!(img = imgCreate(dibHdr.width, dibHdr.height)), "Allocation error\n");

    width = img->width;
    height = img->height;
    rChannel = img->rChannel;
    gChannel = img->gChannel;
    bChannel = img->bChannel;
    nPadding = 4 - (width * 3) % 4;
    nPadding = (nPadding == 4) ? 0 : nPadding;
    nPixels = dibHdr.width * dibHdr.height;
    pixelArrayLen = nPixels * 3 + nPadding * height;

    RET_ERR_MSG(!(pixelArray = malloc(sizeof(uint8_t) * pixelArrayLen)), "Allocation error\n");

    RET_ERR_MSG((nRead = fread(pixelArray, sizeof(uint8_t), pixelArrayLen, f)) != pixelArrayLen,
                "Reading error\n");

    const uint8_t *pixel = pixelArray;
    for (size_t i = 0; i < nPixels; i++, pixel += 3)
    {
        rChannel[i] = pixel[2];
        gChannel[i] = pixel[1];
        bChannel[i] = pixel[0];

        if ((i % width) == width - 1)
        {
            pixel += nPadding;
        }
    }


    // imgDump(img);

    free(pixelArray);
    fclose(f);
    return img;

error:
    if (f) { fclose(f); }
    if (pixelArray) { free(pixelArray); }
    if (img) { imgDestroy(img); }
    return NULL;    
}

bool imgSaveBitmap(const image_t *img, const char *bmpFile)
{
    bmp_hdr_t       bmpHdr;
    dib_hdr_t       dibHdr;
    FILE            *f = NULL;
    size_t          nPixels = 0;
    size_t          nChars = 0;
    const uint8_t   padding = 0x00;
    size_t          nPadding = 0;
    size_t          width = 0;
    size_t          height = 0;
    const uint8_t   *rChannel = NULL;
    const uint8_t   *gChannel = NULL;
    const uint8_t   *bChannel = NULL;

    RET_ERR_MSG(!img, "NULL image\n");
    RET_ERR_MSG(!bmpFile, "NULL file name\n");
    RET_ERR_MSG(!(f = fopen(bmpFile, "wb")), "Failed to open saving file\n");

    width = img->width;
    height = img->height;
    rChannel = img->rChannel;
    gChannel = img->gChannel;
    bChannel = img->bChannel;
    nPixels = width * height;
    nPadding = 4 - (width * 3) % 4;
    nPadding = (nPadding == 4) ? 0 : nPadding;

    bmpHdr.magicNumber = BMP_MAGIC_NUMBER;
    bmpHdr.fileSize = sizeof(bmpHdr) + sizeof(dibHdr) + nPixels * 3 + nPadding * height;
    bmpHdr.reserved1 = 0x0000;
    bmpHdr.reserved2 = 0x0000;
    bmpHdr.pixelsOffset = sizeof(bmpHdr) + sizeof(dibHdr);

    dibHdr.hdrSize = sizeof(dibHdr);
    dibHdr.width = width;
    dibHdr.height = height;
    dibHdr.cPlanes = 1;
    dibHdr.bpp = 24;
    dibHdr.compression = 0;
    dibHdr.imgSize = nPixels * 3 + nPadding * height;
    dibHdr.hResolution = 0x00000000;
    dibHdr.vResolution = 0x00000000;
    dibHdr.nColors = 0x00000000;
    dibHdr.nImpColors = 0x00000000;

#if __BYTE_ORDER == __BIG_ENDIAN 
    bmpHdr.magicNumber = swap16(bmpHdr.magicNumber);
    bmpHdr.fileSize = swap32(bmpHdr.fileSize);
    bmpHdr.reserved1 = swap16(bmpHdr.reserved1);
    bmpHdr.reserved2 = swap16(bmpHdr.reserved2);
    bmpHdr.pixelsOffset = swap32(bmpHdr.pixelsOffset);

    dibHdr.hdrSize = swap32(dibHdr.hdrSize);
    dibHdr.width = swap32(dibHdr.width);
    dibHdr.height = swap32(dibHdr.height);
    dibHdr.cPlanes = swap16(dibHdr.cPlanes);
    dibHdr.bpp = swap16(dibHdr.bpp);
    dibHdr.compression = swap32(dibHdr.compression);
    dibHdr.imgSize = swap32(dibHdr.imgSize);
    dibHdr.hResolution = swap32(dibHdr.hResolution);
    dibHdr.vResolution = swap32(dibHdr.vResolution);
    dibHdr.nColors = swap32(dibHdr.nColors);
    dibHdr.nImpColors = swap32(dibHdr.nImpColors);
#endif

    RET_ERR_MSG((nChars = fwrite(&bmpHdr, sizeof(bmpHdr), 1, f)) != 1, "Write error\n");
    RET_ERR_MSG((nChars = fwrite(&dibHdr, sizeof(dibHdr), 1, f)) != 1, "Write error\n");

    for (size_t r = 0; r < height; r++)
    {
        for (size_t c = 0; c < width; c++)
        {
            RET_ERR_MSG((nChars = fwrite(&imgReadChannel(bChannel, width, r, c),
                                    sizeof(uint8_t), 1, f)) != 1, "Write error\n");
            RET_ERR_MSG((nChars = fwrite(&imgReadChannel(gChannel, width, r, c),
                                    sizeof(uint8_t), 1, f)) != 1, "Write error\n");
            RET_ERR_MSG((nChars = fwrite(&imgReadChannel(rChannel, width, r, c),
                                    sizeof(uint8_t), 1, f)) != 1, "Write error\n");
        }

        /* add padding */
        RET_ERR_MSG((nChars = fwrite(&padding, sizeof(uint8_t), nPadding, f)) != nPadding, "Write error\n");
    }

    fclose(f);
    return true;

error:
    if (f) { fclose(f); }
    return false;
}

bool imgToGrayscale(image_t *img)
{
    size_t      width = 0;
    size_t      height = 0;
    uint8_t     *rChannel = NULL;
    uint8_t     *gChannel = NULL;
    uint8_t     *bChannel = NULL;

    RET_ERR_MSG(!img, "NULL image");

    width = img->width;
    height = img->height;
    rChannel = img->rChannel;
    gChannel = img->gChannel;
    bChannel = img->bChannel;

    for (size_t r = 0; r < height; r++)
    {
        for (size_t c = 0; c < width; c++)
        {
            uint8_t red = imgReadChannel(rChannel, width, r, c);
            uint8_t green = imgReadChannel(gChannel, width, r, c);
            uint8_t blue = imgReadChannel(bChannel, width, r, c);

            uint8_t intensity = 0.2126 * red + 0.7152 * green + 0.0722 * blue;

            imgWriteChannel(rChannel, width, r, c, intensity);
            imgWriteChannel(gChannel, width, r, c, intensity);
            imgWriteChannel(bChannel, width, r, c, intensity);
        }
    }

    return true;

error:
    return false;
}

bool imgToBW(image_t *img)
{
    size_t      width = 0;
    size_t      height = 0;
    uint8_t     *rChannel = NULL;
    uint8_t     *gChannel = NULL;
    uint8_t     *bChannel = NULL;

    RET_ERR_MSG(!img, "NULL image\n");
    RET_ERR_MSG(!imgToGrayscale(img), "Failed to convert to grayscale\n");

    width = img->width;
    height = img->height;
    rChannel = img->rChannel;
    gChannel = img->gChannel;
    bChannel = img->bChannel;

    for (size_t r = 0; r < height; r++)
    {
        for (size_t c = 0; c < width; c++)
        {
            uint8_t intensity = imgReadChannel(rChannel, width, r, c);

            uint8_t val = (intensity > BW_TRASHHOLD) ? 0xFF : 0;

            imgWriteChannel(rChannel, width, r, c, val);
            imgWriteChannel(gChannel, width, r, c, val);
            imgWriteChannel(bChannel, width, r, c, val);
        }
    }

    return true;
error:
    return false;
}

bool imgAvgHash(const image_t *img, uint64_t *res)
{
    image_t         *tmpImg = NULL;
    uint64_t        avgHash = 0x0000000000000000;
    size_t          width = 0;
    const uint8_t   *rChannel = NULL;

    RET_ERR_MSG(!img, "NULL image\n");

    RET_ERR_MSG(!(tmpImg = imgResize(img, AVG_HASH_IMG_DIM, AVG_HASH_IMG_DIM)),
                "Failed to resize image\n");

    RET_ERR_MSG(!imgToGrayscale(tmpImg), "Failed to convert to grayscale\n");
    RET_ERR_MSG(!imgToBW(tmpImg), "Failed to convert to BW\n");

    width = tmpImg->width;
    rChannel = tmpImg->rChannel;

    for (size_t r = 0; r < AVG_HASH_IMG_DIM; r++)
    {
        uint8_t byte = 0x0;
        for (size_t c = 0; c < AVG_HASH_IMG_DIM; c++)
        {
            uint8_t bit = (imgReadChannel(rChannel, width, r, c) == 0) ? 0 : 1;
            byte |= bit << c;
        }

        avgHash |= byte << (r * 8);
    }

    *res = avgHash;
    free (tmpImg);
    return true;

error:
    if (tmpImg) { free(tmpImg); }
    return false;
}

image_t *imgCreate(size_t width, size_t height)
{
    image_t *img = NULL;
    uint8_t *rChannel, *gChannel, *bChannel;
    rChannel = gChannel = bChannel = NULL;

    RET_ERR(!(img = malloc(sizeof(image_t))));

    /* create one memory chunk for all channels */
    RET_ERR(!(rChannel = malloc(sizeof(uint8_t) * width * height * 3)));
    gChannel = rChannel + sizeof(uint8_t) * width * height;
    bChannel = gChannel + sizeof(uint8_t) * width * height;

    img->width = width;
    img->height = height;
    img->rChannel = rChannel;
    img->gChannel = gChannel;
    img->bChannel = bChannel;
    return img;

error:
    if (rChannel) { free(rChannel); }
    if (img) { free(img); }
    return NULL;
}

void imgDestroy(image_t *img)
{
    if (!img)
    {
        return;
    }
    
    img->width = 0;
    img->height = 0;
    free(img->rChannel);
    free(img);
}

void imgDump(const image_t *img)
{
    if (!img)
    {
        return;
    }

    size_t          width = img->width;
    size_t          height = img->height;
    const uint8_t   *rChannel = img->rChannel;
    const uint8_t   *gChannel = img->gChannel;
    const uint8_t   *bChannel = img->bChannel;

    fprintf(stderr, "=== IMAGE ===\n");
    fprintf(stderr, "width:\t%lu\n", width);
    fprintf(stderr, "height:\t%lu\n", height);

    size_t nPixels = (width * height > 50) ? 50 : width * height;
    for (size_t i = 0; i < nPixels; i++)
    {
        fprintf(stderr, "[%3u,%3u,%3u]", rChannel[i], gChannel[i], bChannel[i]);

        if (i % 5 == 4)
        {
            fprintf(stderr, "\n");
        }
    }

    fprintf(stderr, "...\n\n");
}

void dumpBmpHeader(const bmp_hdr_t *bmpHdr)
{
    if (!bmpHdr)
    {
        return;
    }

    fprintf(stderr, "=== BITMAP HEADER ===\n");
    fprintf(stderr, "magicNumber:\t%x\nfileSize:\t%u\nreserved1:\t%u\nreserved2:\t%u\n"
                    "pixelsOffset:\t%u\n",
                    bmpHdr->magicNumber, bmpHdr->fileSize, bmpHdr->reserved1,
                    bmpHdr->reserved2, bmpHdr->pixelsOffset);
    fprintf(stderr, "\n");
}

void dumpDibHeader(const dib_hdr_t *dibHdr)
{
    if (!dibHdr)
    {
        return;
    }

    fprintf(stderr, "=== DIB HEADER ===\n");
    fprintf(stderr, "hdrSize:\t%u\nwidth:\t\t%d\nheight:\t\t%d\ncPlanes:\t%u\nbpp:\t\t%u\n"
                    "compression:\t%u\nimgSize:\t%u\nhResolution:\t%u\nvResolution:\t%u\n"
                    "nColors:\t%u\nnImpColors:\t%u\n",
                    dibHdr->hdrSize, dibHdr->width, dibHdr->height, dibHdr->cPlanes,
                    dibHdr->bpp, dibHdr->compression, dibHdr->imgSize, dibHdr->hResolution,
                    dibHdr->vResolution, dibHdr->nColors, dibHdr->nImpColors);
    fprintf(stderr, "\n");
}