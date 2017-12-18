#include "gtest/gtest.h"
#include "cbmp.h"

const int imageHeight = 2;
const int imageWidth = 2;
const unsigned int imageDepth = 24;
const int RED = 0;
const int GREEN = 1;
const int BLUE = 2;

int imagePixels[imageHeight][imageWidth][imageDepth / 4] = {
        { { 0, 0, 255 }, { 255, 255, 255 } },
        { { 255, 0, 0 }, { 0, 255, 0 } }
};

BMP* bmp;

TEST(CBMPTests, ValidTesting)
{
    EXPECT_TRUE(1);
}

TEST(CBMPTests, OpenImage)
{
    bmp = bopen("../image.bmp");
}

TEST(CBMPTests, CorrectHeight)
{
    EXPECT_EQ(get_height(bmp), imageHeight);
}

TEST(CBMPTests, CorrectWidth)
{
    EXPECT_EQ(get_width(bmp), imageWidth);
}

TEST(CBMPTests, CorrectDepth)
{
    EXPECT_EQ(get_depth(bmp), imageDepth);
}

TEST(CBMPTests, CorrectPixels)
{
    unsigned int x, y, invertedY, width, height;
    width = get_width(bmp);
    height = get_height(bmp);
    unsigned char r, g, b;

    for(y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            get_pixel_rgb(bmp, x, y, &r, &g, &b);

            invertedY = width - y - 1;
            EXPECT_EQ(r, imagePixels[invertedY][x][RED]);
            EXPECT_EQ(g, imagePixels[invertedY][x][GREEN]);
            EXPECT_EQ(b, imagePixels[invertedY][x][BLUE]);
        }
    }
}

TEST(CBMPTests, DeepCopy)
{
    BMP* copy = b_deep_copy(bmp);
    EXPECT_EQ(copy->file_byte_number, bmp->file_byte_number);
    EXPECT_EQ(copy->pixel_array_start, bmp->pixel_array_start);
    EXPECT_EQ(get_width(copy), get_width(bmp));
    EXPECT_EQ(get_height(copy), get_height(bmp));
    EXPECT_EQ(get_depth(copy), get_depth(bmp));

    unsigned int i;
    for (i = 0; i < copy->file_byte_number; i++)
    {
        EXPECT_EQ(copy->file_byte_contents[i], bmp->file_byte_contents[i]);
    }

    EXPECT_NE(copy->file_byte_contents, bmp->file_byte_contents);

    unsigned char r, g, b, cr, cg, cb;
    int x, y;
    for (y = 0; y < get_height(copy); y++)
    {
        for (x = 0; x < get_width(copy); x++)
        {
            get_pixel_rgb(copy, x, y, &cr, &cg, &cb);
            get_pixel_rgb(bmp, x, y, &r, &g, &b);

            EXPECT_EQ(r, cr);
            EXPECT_EQ(g, cg);
            EXPECT_EQ(b, cb);
        }
    }

    EXPECT_NE(copy->pixels, bmp->pixels);
    EXPECT_NE(copy, bmp);

    bclose(copy);
}

TEST(CBMPTests, FreeMemory)
{
    bclose(bmp);
}
