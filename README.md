# C BMP

[![Build Status](https://travis-ci.org/mattflow/cbmp.svg?branch=master)](https://travis-ci.org/mattflow/cbmp)

C Library to read, manipulate, and save BMP images.

Image must be a valid BMP format with 24 or 32 bit pixel data.

## Usage

In order to use the library, move `cbmp.c` and `cbmp.h` into your project.
Link the `cbmp.c` file when compiling and include the `cbmp.h` file at the
top of the file you are using it in.

## Example

This example reads a BMP image and applies an invert to the pixels. It
then saves the inverted image to a new file.

```c
#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"

int main(int argc, char** argv)
{

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(1);
    }

    // Read image into BMP struct
    BMP* bmp = bopen(argv[1]);

    unsigned int x, y, width, height;
    unsigned char r, g, b;

    // Gets image width in pixels
    width = get_width(bmp);

    // Gets image height in pixels
    height = get_height(bmp);

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            // Gets pixel rgb values at point (x, y)
            get_pixel_rgb(bmp, x, y, &r, &g, &b);

            // Sets pixel rgb values at point (x, y)
            set_pixel_rgb(bmp, x, y, 255 - r, 255 - g, 255 - b);
        }
    }

    // Write bmp contents to file
    bwrite(bmp, argv[2]);

    // Free memory
    bclose(bmp);

    return 0;
}
```

## Full API

```c

// Reads a BMP image and returns a poiner to a BMP struct
BMP* bopen(char* file_path);

// Returns a deep copy of the BMP passed
BMP* b_deep_copy(BMP* to_copy);

// Takes a BMP pointer and returns the image width in pixels
int get_width(BMP* bmp);

// Takes a BMP pointer and returns the image height in pixels
int get_height(BMP* bmp);

// Takes a BMP pointer and returns the image depth in bytes
unsigned int get_depth(BMP* bmp);

// Takes a BMP pointer, (x, y), and sets the values of rgb pointers
void get_pixel_rgb(BMP* bmp, int x, int y, unsigned char* r, unsigned char* g, unsigned char* b);

// Takes a BMP pointer, (x, y), and rgb values and updates the pixel color
void set_pixel_rgb(BMP* bmp, int x, int y, unsigned char r, unsigned char g, unsigned char b);

// Writes the contents of the BMP pointer to the given file path
void bwrite(BMP* bmp, char* file_name);

// Frees all memory allocated for the BMP pointer
void bclose(BMP* bmp);

```

## Testing

In order to run tests, create a build directory and enter it:

```sh
mkdir build
cd build
```

Generate `Makefile`s with `cmake`:

```sh
cmake ..
```

Build executable:
```
make
```

Run the `tests` executable:

```sh
./tests
```
