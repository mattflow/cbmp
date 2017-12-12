#include "gtest/gtest.h"
extern "C"
{
  #include "cbmp.h"
}

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

TEST(CBMPTests, FreeMemory)
{
    bclose(bmp);
}
