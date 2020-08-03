#include <assert.h>
#include <errno.h>
#include <string.h>
#include "cbmp.h"

#define EXPECTED_SIZE 78
#define EXPECTED_WIDTH 3
#define EXPECTED_HEIGHT 2

void test__get_file_buffer() {
    printf("test__get_file_buffer: ");
    char* buffer = (char*)_get_file_buffer("input.txt");
    assert(strcmp(buffer, "test") == 0);
    printf("PASSED\n");
}

void test_bmp_open() {
    printf("test_bmp_open: ");
    BMP* bmp = bmp_open("input.bmp");
    // assert(bmp->size == EXPECTED_SIZE);
    // assert(bmp->width == EXPECTED_WIDTH);
    // assert(bmp->height == EXPECTED_HEIGHT);

    BMP* null_bmp = bmp_open("doesnt_exist.bmp");
    assert (null_bmp == NULL);
    assert (errno == ENOENT);

    BMP* not_a_bmp = bmp_open("input.txt");
    assert (not_a_bmp == NULL);
    assert (errno == EINVAL);
    printf("PASSED\n");
}

int main() {
    test__get_file_buffer();
    test_bmp_open();
    return EXIT_SUCCESS;
}