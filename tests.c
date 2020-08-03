#include <assert.h>
#include <errno.h>
#include <string.h>
#include "cbmp.h"

#define DEBUG 1

#if DEBUG
#define LOG(X) printf("%d\n", X);
#else
#define LOG(X)
#endif

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

    BMP* null_bmp = bmp_open("doesnt_exist.bmp");
    assert(null_bmp == NULL);
    assert(errno == ENOENT);

    BMP* not_a_bmp = bmp_open("input.txt");
    assert(not_a_bmp == NULL);
    assert(errno == EINVAL);

    BMP* bmp = bmp_open("input.bmp");
    assert(bmp->file_header->signature == 0x424d);
    assert(bmp->file_header->file_size == 78);
    assert(bmp->file_header->reserved_1 == 0);
    assert(bmp->file_header->reserved_2 == 0);
    assert(bmp->file_header->pixel_offset == 54);

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

    return EXIT_SUCCESS;
}