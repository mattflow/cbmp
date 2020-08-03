#ifndef CBMP_H
#define CBMP_H

/* Includes */

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/* BMP struct not meant to be modified directly */
typedef struct {
    size_t size;
    int width;
    int height;
    int offset;
    int header_size;
    uint8_t* buffer;
} BMP;

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

BMP* _decode_buffer(uint8_t* buffer) {
    BMP* bmp = malloc(sizeof(BMP));
    bmp->buffer = buffer;
    bmp->size = _read_block(buffer, 2, 4);
    bmp->offset = _read_block(buffer, 10, 4);
    bmp->header_size = _read_block(buffer, 14, 4);
    bmp->width = _read_block(buffer, 18, 4);
    bmp->height = _read_block(buffer, 22, 4);
    return bmp;
}

int _get_header_field(uint8_t* buffer) {
    return (buffer[0] << 8) | buffer[1];
}

int _is_valid_header(int header_field) {
    return header_field == 0x424d  /* BM */
        || header_field == 0x4241  /* BA */
        || header_field == 0x4349  /* CI */
        || header_field == 0x4350  /* CP */
        || header_field == 0x4943  /* IC */
        || header_field == 0x5054; /* PT */
}

/* Public functions */

void bmp_close(BMP* bmp) {
    free(bmp->buffer);
    bmp->buffer = NULL;
    free(bmp);
    bmp = NULL;
}

BMP* bmp_open(char* filename) {

    uint8_t* buffer = _get_file_buffer(filename);

    /* There was a problem reading the file. errno was set */
    if (!buffer) {
        return NULL;
    }

    int header_field = _get_header_field(buffer);

    if (!_is_valid_header(header_field)) {
        free(buffer);
        buffer = NULL;
        errno = EINVAL;
        return NULL;
    }

    BMP* bmp = _decode_buffer(buffer);

    return bmp;
}

#endif