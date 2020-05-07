//
// Created by Miguel Alonso Jr on 2019-02-28.
//
#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define BMP_HEADER_SIZE 54
#define DIB_HEADER_SIZE 40


// We need to set the data alignment to 1 by boundary
// because first entry in header is 16 bit
#pragma pack(push)  // save the original data alignment
#pragma pack(1)     // Set data alignment to 1 byte boundary

// uint16_t is a 16-bit unsigned integer
// uint32_t is a 32-bit unsigned integer

typedef struct {
    uint16_t type;              // Magic identifier: 0x4d42
    uint32_t size;              // File size in bytes
    uint16_t reserved1;         // Not used
    uint16_t reserved2;         // Not used
    uint32_t offset;            // Offset to image data in bytes from beginning of file
    uint32_t dib_header_size;   // DIB Header size in bytes
    int32_t  width_px;          // Width of the image
    int32_t  height_px;         // Height of image
    uint16_t num_planes;        // Number of color planes
    uint16_t bits_per_pixel;    // Bits per pixel
    uint32_t compression;       // Compression type
    uint32_t image_size_bytes;  // Image size in bytes
    int32_t  x_resolution_ppm;  // Pixels per meter
    int32_t  y_resolution_ppm;  // Pixels per meter
    uint32_t num_colors;        // Number of colors
    uint32_t important_colors;  // Important colors
} BitmapHeader;


#pragma pack(pop)  // restore the previous pack setting

typedef struct {
    BitmapHeader header;
    unsigned char *data;
} Bitmap;

Bitmap* read_bitmap(char *filename, char **error);
bool write_bitmap(const char *filename, Bitmap *image, char **error);
bool check_bitmap_header(BitmapHeader header, char *filename);

#endif //BITMAP_H

