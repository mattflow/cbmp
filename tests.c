#include <assert.h>
#include <errno.h>
#include <string.h>
#include "cbmp.h"

#define EXPECTED_SIZE 78
#define EXPECTED_WIDTH 3
#define EXPECTED_HEIGHT 2

void test__get_file_buffer() {
    printf("test__get_file_buffer: ");
    char* expected = "test";
    int expected_length = strlen(expected);
    char* buffer = (char*)_get_file_buffer("input.txt");
    int i;
    for (i = 0; i < expected_length; i++) {
        assert(buffer[i] == expected[i]);
    }
    free(buffer);
    buffer = NULL;
    printf("PASSED\n");
}

BMP* test_bmp_open() {
    printf("test_bmp_open: ");
    BMP* bmp = bmp_open("input.bmp");
    /* assert(bmp->size == EXPECTED_SIZE); */
    /* assert(bmp->width == EXPECTED_WIDTH); */
    /* assert(bmp->height == EXPECTED_HEIGHT); */

    BMP* null_bmp = bmp_open("doesnt_exist.bmp");
    assert (null_bmp == NULL);
    assert (errno == ENOENT);

    BMP* not_a_bmp = bmp_open("input.txt");
    assert (not_a_bmp == NULL);
    assert (errno == EINVAL);
    printf("PASSED\n");

    return bmp;
}

/* This will trigger a valgrind error if failing */
void test_bmp_close(BMP* bmp) {
    bmp_close(bmp);
}

int main() {
    /* Test private functions */
    test__get_file_buffer();

    /* Test public functions */
    BMP* bmp = test_bmp_open();
    test_bmp_close(bmp);
    assert(1 == 0);
    return EXIT_SUCCESS;
}