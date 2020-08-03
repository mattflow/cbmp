#ifndef CBMP_H
#define CBMP_H

/* Includes */

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/* BMP struct not meant to be modified directly */

typedef struct {
    int signature;
    int file_size;
    int reserved_1;
    int reserved_2;
    int pixel_offset;
} FileHeader;

typedef struct {
    int header_size;
    int width;
    int height;
    int color_planes;
    int bits_per_pixel;
    int compression_method;
    int image_size;
    int horizontal_resolution;
    int vertical_resolution;
    int color_palette;
    int important_colors;
    int resolution_units;
    int padding;
    int bit_direction;
    int halftoning_algorithm;
    int halftoning_parameter_1;
    int halftoning_parameter_2;
    int color_encoding;
    int application_defined_identifier;
} DIBHeader;

typedef struct {
    FileHeader* file_header;
    DIBHeader* dib_header;
    uint8_t* buffer;
} BMP;

/* Public declarations */

BMP* bmp_open(char* filename);
void bmp_close(BMP* bmp);
int bmp_get_width(BMP* bmp);
int bmp_get_height(BMP* bmp);

/* Private functions */

/* Returns NULL and sets errno if there was a problem reading the file */
uint8_t* _get_file_buffer(char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }

    int fseek_result = fseek(file, 0, SEEK_END);
    if (fseek_result != 0) {
        fclose(file);
        return NULL;
    }

    long size = ftell(file);
    if (size == -1L) {
        fclose(file);
        return NULL;
    }

    rewind(file);
    uint8_t* buffer = malloc(sizeof(uint8_t) * size);
    size_t fread_result = fread(buffer, sizeof(uint8_t), size, file);
    fclose(file);

    if (fread_result != (size_t)size) {
        free(buffer);
        buffer = NULL;
        return NULL;
    }

    return buffer;
}

int _read_block(uint8_t* buffer, int offset, int size) {
    int result = 0;
    int i;
    for (i = 0; i < size; i++) {
        int index = offset + i;
        result = result | (buffer[index] << (i * 8));
    }
    return result;
}

int _get_header_signature(uint8_t* buffer) {
    return (buffer[0] << 8) | buffer[1];
}

int _is_valid_signature(int signature) {
    return signature == 0x424d  /* BM */
        || signature == 0x4241  /* BA */
        || signature == 0x4349  /* CI */
        || signature == 0x4350  /* CP */
        || signature == 0x4943  /* IC */
        || signature == 0x5054; /* PT */
}

FileHeader* _read_file_header(uint8_t* buffer) {
    FileHeader* file_header = malloc(sizeof(FileHeader));
    int signature = _get_header_signature(buffer);
    if (!_is_valid_signature(signature)) {
        free(file_header);
        file_header = NULL;
        fprintf(stderr, "%0x: Unsupported file type\n", signature);
        return NULL;
    }
    file_header->signature = signature;
    file_header->file_size = _read_block(buffer, 2, 4);
    file_header->reserved_1 = _read_block(buffer, 6, 2);
    file_header->reserved_2 = _read_block(buffer, 8, 2);
    file_header->pixel_offset = _read_block(buffer, 10, 4);
    return file_header;
}

DIBHeader* _read_BITMAPINFOHEADER(uint8_t* buffer) {
    DIBHeader* dib_header = malloc(sizeof(DIBHeader));
    dib_header->width = _read_block(buffer, 18, 4);
    dib_header->height = _read_block(buffer, 22, 4);
    dib_header->color_planes = _read_block(buffer, 26, 2);
    dib_header->bits_per_pixel = _read_block(buffer, 28, 2);
    dib_header->compression_method = _read_block(buffer, 30, 4);
    dib_header->image_size = _read_block(buffer, 34, 4);
    dib_header->horizontal_resolution = _read_block(buffer, 38, 4);
    dib_header->vertical_resolution = _read_block(buffer, 42, 4);
    dib_header->color_palette = _read_block(buffer, 46, 4);
    dib_header->important_colors = _read_block(buffer, 50, 4);
    return dib_header;
}

DIBHeader* _read_dib_header(uint8_t* buffer) {
    DIBHeader* dib_header = NULL;

    int header_size = _read_block(buffer, 14, 4);

    switch(header_size) {
        case 40: /* BITMAPINFOHEADER */
            dib_header = _read_BITMAPINFOHEADER(buffer);
            break;
        case 12:
        case 64:
        case 16:
        case 52:
        case 56:
        case 108:
        case 124:
        default:
            fprintf(stderr, "%d: BMP format not supported\n", header_size);
            return NULL;
    }

    dib_header->header_size = header_size;

    return dib_header;
}

BMP* _decode_buffer(uint8_t* buffer) {
    BMP* bmp = malloc(sizeof(BMP));
    bmp->file_header = NULL;
    bmp->dib_header = NULL;

    bmp->buffer = buffer;

    bmp->file_header = _read_file_header(buffer);
    if (!bmp->file_header) {
        bmp_close(bmp);
        return NULL;
    }

    bmp->dib_header = _read_dib_header(buffer);
    if (!bmp->dib_header) {
        bmp_close(bmp);
        return NULL;
    }

    switch(bmp->dib_header->compression_method) {
        case 0:
        case 11:
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 12:
        case 13:
        default:
            fprintf(stderr, "%d: Compression type is not supported\n", bmp->dib_header->compression_method);
            bmp_close(bmp);
            return NULL;
    }

    return bmp;
}

void _free(void* ptr) {
    if (ptr) {
        free(ptr);
        ptr = NULL;
    }
}

/* Public implementations */

BMP* bmp_open(char* filename) {

    uint8_t* buffer = _get_file_buffer(filename);

    /* There was a problem reading the file. errno was set */
    if (!buffer) {
        return NULL;
    }

    BMP* bmp = _decode_buffer(buffer);

    if (!bmp) {
        errno = EINVAL;
        return NULL;
    }

    return bmp;
}

int bmp_get_width(BMP* bmp) {
    return bmp->dib_header->width;
}

int bmp_get_height(BMP* bmp) {
    return bmp->dib_header->height;
}

void bmp_close(BMP* bmp) {
    _free(bmp->file_header);
    _free(bmp->dib_header);
    _free(bmp->buffer);
    _free(bmp);
}

#endif