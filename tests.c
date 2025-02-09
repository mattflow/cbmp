#include <stdio.h>
#include "cbmp.h"

int main()
{

    FILE *file = fopen("input.bmp", "rb");
    fclose(file);
    say_hi();
    return 0;
}