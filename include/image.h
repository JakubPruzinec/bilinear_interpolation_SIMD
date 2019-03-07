#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "utils.h"


#define BMP_MAGIC_NUMBER    0x4D42

#define BW_TRASHHOLD        127

#define AVG_HASH_IMG_DIM    8
#define AVG_HASH_SIMILARITY_TRASHHOLD   3


/* Uniform prepresentation of a single image */
typedef struct
{
    size_t width;
    size_t height;
    uint8_t *rChannel;
    uint8_t *gChannel;
    uint8_t *bChannel;
} image_t;

/* BMP header */
struct bmp_hdr
{
    uint16_t magicNumber;               ///< magic number of BMP (0x42 0x4d 'B' 'M')
    uint32_t fileSize;                  ///< file size in bytes
    uint16_t reserved1;                 ///< reserved (generaly ignored)
    uint16_t reserved2;                 ///< reserved (generaly ignored)
    uint32_t pixelsOffset;              ///< offset to pixel array
} __attribute__((packed));

typedef struct bmp_hdr bmp_hdr_t;


/* DIB header */
struct dib_hdr
{
    uint32_t hdrSize;                   ///< header size (must be 40)
    int32_t width;                      ///< image width (signed)
    int32_t height;                     ///< image height (signed)
    uint16_t cPlanes;                   ///< color planes (must be 1, generally ignored)
    uint16_t bpp;                       ///< bits per pixel
    uint32_t compression;               ///< compression method
    uint32_t imgSize;                   ///< image size in bytes
    uint32_t hResolution;               ///< horizontal resolution (generally ignored)
    uint32_t vResolution;               ///< vertical resolution (generally ignored)
    uint32_t nColors;                   ///< number of collors in color palette
    uint32_t nImpColors;                ///< number of important colors (generally ignored)
} __attribute__((packed));

typedef struct dib_hdr dib_hdr_t;

/**
 * Load BMP image from a file
 * @param bmpFile Name of file to parse image from
 * @return Parsed image or NULL on error
 */
image_t *imgLoadBitmap(const char *bmpFile);

/**
 * Save image
 * @param img Image to save
 * @param bmpFile Name of file to save image to
 * @return Success flag
 */
bool imgSaveBitmap(const image_t *img, const char *bmpFile);

/**
 * Read channel value at given possition. Bounds are not checked
 * @param channel Pointer to channel array
 * @param width Image width
 * @param r Row (indexed from 0)
 * @param c Column (indexed from 0)
 * @return Channel value
 */
#define imgReadChannel(channel, width, r, c)        \
    channel[(r) * (width) + (c)]

/**
 * Write channel value at given possition. Bounds are not checked
 * @param channel Pointer to channel array
 * @param width Image width
 * @param r Row (indexed from 0)
 * @param c Column (indexed from 0)
 * @param v Value to write
 * @return Written value
 */
#define imgWriteChannel(channel, width, r, c, v)    \
    (channel)[(r) * (width) + (c)] = (v)

/**
 * Resize image with bilinear interpolation, create a NEW image
 * @param img Image to resize
 * @param newWidth Width of resized image
 * @param newHeight Height of resized image
 * @return New image or NULL on error
 */
image_t *imgResize(const image_t *img, size_t newWidth, size_t newHeight);

/**
 * Convert image to greyscale
 * @param img Image to convert
 * @return Success flag
 */
bool imgToGrayscale(image_t *img);

/**
 * Converts image to black and white
 * @param img Image to convert
 * @return Success flag
 */
bool imgToBW(image_t *img);

/**
 * Computes average hash of image
 * @param img Image to compute the avg hash for
 * @param res Variable to store the hash to.
 * @return Success flag
 */
bool imgAvgHash(const image_t *img, uint64_t *res);

/**
 * Allocates necessary resources for image
 * @param width Image width
 * @param height Image height
 * @return New image or NULL on error
 */
image_t *imgCreate(size_t width, size_t height);

/**
 * Deallocates all resources of an image
 * @param img Image to destroy
 */
void imgDestroy(image_t *img);

/**
 * Dumps image
 * @param img Image to dump
 */
void imgDump(const image_t *img);

/**
 * Dump BMP header to stderr
 * @param bmpHdr Bmp header
 */
void dumpBmpHeader(const bmp_hdr_t *bmpHdr);

/**
 * Dump DIB header to stderr
 * @param dibHdr Dib header
 */
void dumpDibHeader(const dib_hdr_t *dibHdr);

#endif // guardian