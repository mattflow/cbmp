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
    uint8_t* buffer = (uint8_t*)malloc(sizeof(uint8_t) * size);
    size_t fread_result = fread(buffer, sizeof(uint8_t), size, file);

    if (fread_result != (size_t)size) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    return buffer;
}

/* Public functions */

BMP* bmp_open(char* filename) {

    uint8_t* buffer = _get_file_buffer(filename);

    /* There was a problem reading the file. errno was set */
    if (!buffer) {
        return NULL;
    }

    BMP* bmp = (BMP*)malloc(sizeof(BMP));
    bmp->buffer = buffer;

    /* First two bytes of BMP files are B and M respectively */
    if (bmp->buffer[0] != 'B' || bmp->buffer[1] != 'M') {
        /* Invalid argument */
        errno = EINVAL;
        return NULL;
    }

    return bmp;
}



#endif