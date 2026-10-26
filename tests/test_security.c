// Regression tests for the integer-overflow / OOB disclosure.
//
// Malicious inputs must be rejected (the library calls _throw_error, which
// exits non-zero) rather than overflowing the heap. A valid image must still
// open. We run each malicious case in a forked child so one exit() doesn't
// end the whole suite, and assert the child died via exit (not a signal/crash).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../cbmp.h"

static void write_file(const char* path, const unsigned char* data, size_t n)
{
    FILE* fp = fopen(path, "wb");
    if (!fp) { perror("fopen"); exit(2); }
    fwrite(data, 1, n, fp);
    fclose(fp);
}

// Expect bopen(path) to reject the file cleanly: child must exit() with a
// non-zero status, never die from a signal (SIGSEGV/SIGABRT = still exploitable).
static int expect_rejected(const char* name, const char* path)
{
    fflush(stdout); // avoid the forked child re-flushing our buffered output
    pid_t pid = fork();
    if (pid == 0)
    {
        fclose(stderr); // silence the expected error message
        BMP* bmp = bopen(path);
        bclose(bmp);
        _exit(0); // reached only if bopen wrongly succeeded
    }
    int status = 0;
    waitpid(pid, &status, 0);

    if (WIFSIGNALED(status))
    {
        printf("  FAIL %s: crashed with signal %d (still vulnerable)\n", name, WTERMSIG(status));
        return 1;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
    {
        printf("  FAIL %s: file was accepted, expected rejection\n", name);
        return 1;
    }
    printf("  ok   %s: rejected (exit %d)\n", name, WEXITSTATUS(status));
    return 0;
}

int main(void)
{
    int failures = 0;

    // --- The disclosed PoC: 58-byte file, width=0x10000, height=0x10001 ---
    unsigned char poc[58] = {0};
    poc[0] = 'B'; poc[1] = 'M';
    poc[2] = 0x3a;                 // file size field (cosmetic)
    poc[10] = 0x36;                // pixel_array_start = 54
    poc[14] = 0x28;                // DIB header size
    // width = 0x00010000 at offset 18
    poc[18]=0x00; poc[19]=0x00; poc[20]=0x01; poc[21]=0x00;
    // height = 0x00010001 at offset 22
    poc[22]=0x01; poc[23]=0x00; poc[24]=0x01; poc[25]=0x00;
    poc[26]=0x01;                  // planes
    poc[28]=0x20;                  // depth = 32
    write_file("poc_overflow.bmp", poc, sizeof(poc));
    failures += expect_rejected("overflow_poc", "poc_overflow.bmp");

    // --- Truncated file: valid magic but too small for the header ---
    unsigned char tiny[4] = {'B','M',0,0};
    write_file("tiny.bmp", tiny, sizeof(tiny));
    failures += expect_rejected("truncated_header", "tiny.bmp");

    // --- Zero dimensions ---
    unsigned char zero[58] = {0};
    zero[0]='B'; zero[1]='M'; zero[10]=0x36; zero[28]=0x18; // depth 24, w=h=0
    write_file("zero.bmp", zero, sizeof(zero));
    failures += expect_rejected("zero_dimensions", "zero.bmp");

    // --- Dimensions larger than the file can back (OOB read guard) ---
    unsigned char big[58] = {0};
    big[0]='B'; big[1]='M'; big[10]=0x36;
    big[18]=0x00; big[19]=0x01;    // width = 256
    big[22]=0x00; big[23]=0x01;    // height = 256
    big[28]=0x18;                  // depth 24 -> needs ~196KB, file is 58 bytes
    write_file("big.bmp", big, sizeof(big));
    failures += expect_rejected("data_past_eof", "big.bmp");

    // --- A genuinely valid 2x2 24-bit image must still open ---
    {
        // row_size = ((24*2+31)/32)*4 = 8 bytes/row; 2 rows = 16 bytes of pixels
        unsigned int w = 2, h = 2, start = 54;
        unsigned int fsize = start + 16;
        unsigned char* ok = calloc(fsize, 1);
        ok[0]='B'; ok[1]='M';
        ok[10]=(unsigned char)start;
        ok[14]=0x28;
        ok[18]=(unsigned char)w;
        ok[22]=(unsigned char)h;
        ok[26]=1;
        ok[28]=0x18; // depth 24
        write_file("valid.bmp", ok, fsize);
        free(ok);

        BMP* bmp = bopen("valid.bmp");
        if (get_width(bmp) == (int)w && get_height(bmp) == (int)h)
            printf("  ok   valid_image: opened %dx%d\n", get_width(bmp), get_height(bmp));
        else { printf("  FAIL valid_image: wrong dimensions\n"); failures++; }
        bclose(bmp);
    }

    if (failures)
    {
        printf("\n%d test(s) FAILED\n", failures);
        return 1;
    }
    printf("\nAll tests passed\n");
    return 0;
}
