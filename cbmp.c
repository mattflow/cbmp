#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "cbmp.h"

// Constants

#define BITS_PER_BYTE 8

#define BLUE 0
#define GREEN 1
#define RED 2
#define ALPHA 3

#define PIXEL_ARRAY_START_BYTES 4
#define PIXEL_ARRAY_START_OFFSET 10

#define WIDTH_BYTES 4
#define WIDTH_OFFSET 18

#define HEIGHT_BYTES 4
#define HEIGHT_OFFSET 22

#define DEPTH_BYTES 2
#define DEPTH_OFFSET 28

// Smallest file we can safely parse: every header field we read lives
// below this offset (the last is depth at bytes 28-29).
#define MIN_HEADER_BYTES (DEPTH_OFFSET + DEPTH_BYTES)

// Private function declarations

void _throw_error(char* message);
void _validate_dimensions(BMP* bmp);
unsigned int _get_int_from_buffer(unsigned int bytes, 
                                  unsigned int offset, 
                                  unsigned char* buffer);
unsigned int _get_file_byte_number(FILE* fp);
unsigned char* _get_file_byte_contents(FILE* fp, unsigned int file_byte_number);
int _validate_file_type(unsigned char* file_byte_contents);
int _validate_depth(unsigned int depth);
unsigned int _get_pixel_array_start(unsigned char* file_byte_contents);
int _get_width(unsigned char* file_byte_contents);
int _get_height(unsigned char* file_byte_contents);
unsigned int _get_depth(unsigned char* file_byte_contents);
void _update_file_byte_contents(BMP* bmp, int index, int offset, int channel);
void _populate_pixel_array(BMP* bmp);
void _map(BMP* bmp, void (*f)(BMP* bmp, int, int, int));
void _get_pixel(BMP* bmp, int index, int offset, int channel);

// Public function implementations

BMP* bopen(char* file_path)
{
    FILE* fp = fopen(file_path, "rb");
  
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    BMP* bmp = (BMP*) malloc(sizeof(BMP));
    if (bmp == NULL)
    {
        _throw_error("Failed to allocate memory");
    }
    bmp->file_byte_number = _get_file_byte_number(fp);
    bmp->file_byte_contents = _get_file_byte_contents(fp, bmp->file_byte_number);
    fclose(fp);

    // Reject files too small to hold the header fields we read below,
    // otherwise _validate_file_type and the header getters read out of bounds.
    if (bmp->file_byte_number < MIN_HEADER_BYTES)
    {
        _throw_error("File too small to be a valid BMP");
    }

    if(!_validate_file_type(bmp->file_byte_contents))
    {
        _throw_error("Invalid file type");
    }

    bmp->pixel_array_start = _get_pixel_array_start(bmp->file_byte_contents);

    bmp->width = _get_width(bmp->file_byte_contents);
    bmp->height = _get_height(bmp->file_byte_contents);
    bmp->depth = _get_depth(bmp->file_byte_contents);

    if(!_validate_depth(bmp->depth))
    {
        _throw_error("Invalid file depth");
    }

    // Reject dimensions that would overflow the pixel allocation or point
    // the pixel reads past the end of the file (CWE-190 / CWE-122 / CWE-126).
    _validate_dimensions(bmp);

    _populate_pixel_array(bmp);

    return bmp;
}

BMP* b_deep_copy(BMP* to_copy)
{
    BMP* copy = (BMP*) malloc(sizeof(BMP));
    if (copy == NULL)
    {
        _throw_error("Failed to allocate memory");
    }
    copy->file_byte_number = to_copy->file_byte_number;
    copy->pixel_array_start = to_copy->pixel_array_start;
    copy->width = to_copy->width;
    copy->height = to_copy->height;
    copy->depth = to_copy->depth;

    copy->file_byte_contents = (unsigned char*) malloc(copy->file_byte_number * sizeof(unsigned char));
    if (copy->file_byte_contents == NULL)
    {
        _throw_error("Failed to allocate memory");
    }

    unsigned int i;
    for (i = 0; i < copy->file_byte_number; i++)
    {
        copy->file_byte_contents[i] = to_copy->file_byte_contents[i];
    }

    // Dimensions were already validated when to_copy was opened, so this
    // product cannot overflow; still guard the allocation failing.
    copy->pixels = (pixel*) malloc(copy->width * copy->height * sizeof(pixel));
    if (copy->pixels == NULL)
    {
        _throw_error("Failed to allocate memory");
    }

    unsigned int x, y;
    int index;
    for (y = 0; y < copy->height; y++)
    {
        for (x = 0; x < copy->width; x++)
        {
            index = y * copy->width + x;
            copy->pixels[index].red = to_copy->pixels[index].red;
            copy->pixels[index].green = to_copy->pixels[index].green;
            copy->pixels[index].blue = to_copy->pixels[index].blue;
            copy->pixels[index].alpha = to_copy->pixels[index].alpha;
        }
    }

    return copy;
}

int get_width(BMP* bmp)
{
    return bmp->width;
}

int get_height(BMP* bmp)
{
    return bmp->height;
}

unsigned int get_depth(BMP* bmp)
{
    return bmp->depth;
}

void get_pixel_rgb(BMP* bmp, int x, int y, unsigned char* r, unsigned char* g, unsigned char* b)
{
    int index = y * bmp->width + x;
    *r = bmp->pixels[index].red;
    *g = bmp->pixels[index].green;
    *b = bmp->pixels[index].blue;
}

void set_pixel_rgb(BMP* bmp, int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
    int index = y * bmp->width + x;
    bmp->pixels[index].red = r;
    bmp->pixels[index].green = g;
    bmp->pixels[index].blue = b;
}

void bwrite(BMP* bmp, char* file_name)
{
    _map(bmp, _update_file_byte_contents);

    FILE* fp = fopen(file_name, "wb");
    fwrite(bmp->file_byte_contents, sizeof(char), bmp->file_byte_number, fp);
    fclose(fp);
}

void bclose(BMP* bmp)
{
    free(bmp->pixels);
    bmp->pixels = NULL;
    free(bmp->file_byte_contents);
    bmp->file_byte_contents = NULL;
    free(bmp);
    bmp = NULL;
}


// Private function implementations

void _throw_error(char* message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

unsigned int _get_int_from_buffer(unsigned int bytes, 
                                  unsigned int offset, 
                                  unsigned char* buffer)
{
    unsigned int value = 0;
    int i;

    for (i = bytes - 1; i >= 0; --i)
    {
        value <<= 8;
        value += buffer[i + offset];
    }

    return value;
}

unsigned int _get_file_byte_number(FILE* fp)
{
    unsigned int byte_number;
    fseek(fp, 0, SEEK_END);
    byte_number = ftell(fp);
    rewind(fp);
    return byte_number;
}

unsigned char* _get_file_byte_contents(FILE* fp, unsigned int file_byte_number)
{
    unsigned char* buffer = (unsigned char*) malloc(file_byte_number * sizeof(char));
    if (buffer == NULL)
    {
        _throw_error("Failed to allocate memory");
    }
    unsigned int result = fread(buffer, 1, file_byte_number, fp);

    if (result != file_byte_number)
    {
        _throw_error("There was a problem reading the file");
    }


    return buffer;
}

int _validate_file_type(unsigned char* file_byte_contents)
{
    return file_byte_contents[0] == 'B' && file_byte_contents[1] == 'M';
}

int _validate_depth(unsigned int depth)
{
    return depth == 24 || depth == 32;
}

unsigned int _get_pixel_array_start(unsigned char* file_byte_contents)
{
    return _get_int_from_buffer(PIXEL_ARRAY_START_BYTES, PIXEL_ARRAY_START_OFFSET, file_byte_contents);
}

int _get_width(unsigned char* file_byte_contents)
{
    return (int) _get_int_from_buffer(WIDTH_BYTES, WIDTH_OFFSET, file_byte_contents);
}

int _get_height(unsigned char* file_byte_contents)
{
    return (int) _get_int_from_buffer(HEIGHT_BYTES, HEIGHT_OFFSET, file_byte_contents);
}

unsigned int _get_depth(unsigned char* file_byte_contents)
{
    return _get_int_from_buffer(DEPTH_BYTES, DEPTH_OFFSET, file_byte_contents);
}

void _update_file_byte_contents(BMP* bmp, int index, int offset, int channel)
{
    char value;
    switch(channel)
    {
        case BLUE:
            value = bmp->pixels[index].blue;
            break;
        case GREEN:
            value = bmp->pixels[index].green;
            break;
        case RED:
            value = bmp->pixels[index].red;
            break;
        case ALPHA:
            value = bmp->pixels[index].alpha;
            break;
    }
    bmp->file_byte_contents[offset + channel] = value;
}

void _validate_dimensions(BMP* bmp)
{
    if (bmp->width == 0 || bmp->height == 0)
    {
        _throw_error("Invalid image dimensions");
    }

    size_t width = bmp->width;
    size_t height = bmp->height;
    size_t depth = bmp->depth;
    size_t start = bmp->pixel_array_start;

    // width * height * sizeof(pixel) must not overflow size_t (CWE-190).
    // Checking in size_t also covers the 32-bit unsigned int wrap that the
    // original `width * height` multiply suffered from.
    if (width > SIZE_MAX / height ||
        width * height > SIZE_MAX / sizeof(pixel))
    {
        _throw_error("Image dimensions too large");
    }

    // Each row is padded to a multiple of 4 bytes: ((depth*width + 31)/32)*4.
    // Guard the multiply so the bounds check below cannot itself overflow.
    if (width > (SIZE_MAX - 31) / depth)
    {
        _throw_error("Image dimensions too large");
    }
    size_t row_size = ((depth * width + 31) / 32) * 4;

    // Every pixel read in _map stays below start + row_size*height, so require
    // the file to actually hold that many bytes (CWE-126 out-of-bounds read).
    if (row_size != 0 && height > (SIZE_MAX - start) / row_size)
    {
        _throw_error("Image dimensions too large");
    }
    if (start + row_size * height > bmp->file_byte_number)
    {
        _throw_error("Pixel data extends past end of file");
    }
}

void _populate_pixel_array(BMP* bmp)
{
    bmp->pixels = (pixel*) malloc(bmp->width * bmp->height * sizeof(pixel));
    if (bmp->pixels == NULL)
    {
        _throw_error("Failed to allocate memory");
    }
    _map(bmp, _get_pixel);
}

void _map(BMP* bmp, void (*f)(BMP*, int, int, int))
{
    int channels = bmp->depth / (sizeof(unsigned char) * BITS_PER_BYTE);
    int row_size = ((int) (bmp->depth * bmp->width + 31) / 32) * 4;
    int padding = row_size - bmp->width * channels;

    int c;
    unsigned int x, y, index, offset;
    for (y = 0; y < bmp->height; y++)
    {
        for (x = 0; x < bmp->width; x++)
        {
            index = y * bmp->width + x;
            offset = bmp->pixel_array_start + index * channels + y * padding;
            for (c = 0; c < channels; c++)
            {
                (*f)(bmp, index, offset, c);
            }
        }
    }
}

void _get_pixel(BMP* bmp, int index, int offset, int channel)
{
    unsigned char value = _get_int_from_buffer(sizeof(unsigned char), offset + channel, bmp->file_byte_contents);
    switch(channel)
    {
        case BLUE:
            bmp->pixels[index].blue = value;
            break;
        case GREEN:
            bmp->pixels[index].green = value;
            break;
        case RED:
            bmp->pixels[index].red = value;
            break;
        case ALPHA:
            bmp->pixels[index].alpha = value;
            break;
    }
}
