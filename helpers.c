#include "helpers.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <cs50.h>

// Globals
int gxKernel[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
int gyKernel[3][3] = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

int widthGlobal;
int heightGlobal;
int numberOfPixelsSummed = 0;

/* RGBTEIPLE struct only has a byte as a datatype, so it cannot exceed
a value of 255. The int and double structs were made so the number
could exceed that for calculations.  Structs were made to keep values together. */
typedef struct
{
    int  rgbtBlue;
    int  rgbtGreen;
    int  rgbtRed;
} __attribute__((__packed__))
RGBTRIPLEInt;

typedef struct
{
    double  rgbtBlue;
    double  rgbtGreen;
    double  rgbtRed;
} __attribute__((__packed__))
RGBTRIPLEDouble;

// Prototypes for Functions
void SetWidthAndHeightGlobalVariables(int widthVar, int heightVar);
void LoopThroughPixels(string filter, RGBTRIPLE(*image)[widthGlobal]);
RGBTRIPLE ApplyGreyscaleFilterToPixel(RGBTRIPLE pixel);
RGBTRIPLE ApplyHorizontalRelfectionToPixel(int row, int column, RGBTRIPLE(*image)[widthGlobal]);
RGBTRIPLE LoopThrough3by3GridAroundPixel(string filter, int row, int column, RGBTRIPLE(*image)[widthGlobal]);
RGBTRIPLEInt AddCurrentValuesToSumIntPixel(RGBTRIPLE currentPixel, RGBTRIPLEInt sumPixel);
RGBTRIPLEDouble AddCurrentValuesToSumDoublePixel(RGBTRIPLE currentPixel, RGBTRIPLEDouble sumDoublePixel);
RGBTRIPLE AverageValuesOfSumIntPixel(RGBTRIPLEInt sumPixel);
RGBTRIPLEDouble GetGValuesForCurrentPixelAndAddToSum(RGBTRIPLE currentPixel, int gKernelValue, RGBTRIPLEDouble sumDoublePixelG);
RGBTRIPLEDouble AddGValuesToSumDoublePixel(RGBTRIPLEDouble doublePixelG, RGBTRIPLEDouble sumDoublePixel);
RGBTRIPLEDouble GetGValuesForCurrentPixel(RGBTRIPLE currentPixel, int gKernelValue);
RGBTRIPLE MergeGXAndGYPixels(RGBTRIPLEDouble GX, RGBTRIPLEDouble GY);
int GetRoundedAverage(int sum, float numberOfItems);
void copyRGBTRIPLEArray(RGBTRIPLE(*source)[widthGlobal], RGBTRIPLE(*copy)[widthGlobal]);

/* The first 4 functions are only here because they were pre-made as part of the problem and cannot be changed.
    These could have been called directly from the switch statement in filter.c */
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    SetWidthAndHeightGlobalVariables(width, height);
    LoopThroughPixels("grayscale", image);

    return;
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    SetWidthAndHeightGlobalVariables(width, height);
    LoopThroughPixels("horizontalReflection", image);

    return;
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    SetWidthAndHeightGlobalVariables(width, height);
    LoopThroughPixels("blur", image);

    return;
}

// Find edges of image
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    /* This is only here because it has to be due to the way the problem was setup.
    These could have been called directly from the switch statement in filter.c */
    SetWidthAndHeightGlobalVariables(width, height);
    LoopThroughPixels("edges", image);

    return;
}

void SetWidthAndHeightGlobalVariables(int widthVar, int heightVar)
{
    /* Set the variables as a global so that they don't
    have to be passed into each function, they don't change*/
    widthGlobal = widthVar;
    heightGlobal = heightVar;
}

void LoopThroughPixels(string filter, RGBTRIPLE(*image)[widthGlobal])
{
    RGBTRIPLE tempImage[heightGlobal][widthGlobal];
    copyRGBTRIPLEArray(image, tempImage);

    // For each row
    for (int row = 0; row < heightGlobal; row++)
    {
        // For each pixel in that row
        for (int column = 0; column < widthGlobal; column++)
        {
            if (strcmp(filter, "grayscale") == 0)
            {
                tempImage[row][column] = ApplyGreyscaleFilterToPixel(image[row][column]);
            }
            else if (strcmp(filter, "horizontalReflection") == 0)
            {
                tempImage[row][column] = ApplyHorizontalRelfectionToPixel(row, column, image);
            }
            else if (strcmp(filter, "blur") == 0)
            {
                tempImage[row][column] = LoopThrough3by3GridAroundPixel("blur", row, column, image);
            }
            else if (strcmp(filter, "edges") == 0)
            {
                tempImage[row][column] = LoopThrough3by3GridAroundPixel("edges", row, column, image);
            }
            else
            {
                // Future-proofing. The current code should never get to here.
                printf("Filter Not Chosen.  Please check commandline arguments.\n");
                break;
            }

        }
    }

    copyRGBTRIPLEArray(tempImage, image);

    return;
}

RGBTRIPLE ApplyGreyscaleFilterToPixel(RGBTRIPLE pixel)
{
    BYTE averageRGB = (int)round((pixel.rgbtRed + pixel.rgbtGreen + pixel.rgbtBlue) / 3.0);

    pixel.rgbtRed = averageRGB;
    pixel.rgbtBlue = averageRGB;
    pixel.rgbtGreen = averageRGB;

    return pixel;
}

RGBTRIPLE ApplyHorizontalRelfectionToPixel(int row, int column, RGBTRIPLE(*image)[widthGlobal])
{
    int mirrorLocation = (int)(widthGlobal - 1 - column);
    return image[row][mirrorLocation];
}

RGBTRIPLE LoopThrough3by3GridAroundPixel(string filter, int row, int column, RGBTRIPLE(*image)[widthGlobal])
{
    // Initialize blank pixels to avoid garbage values.
    RGBTRIPLEInt sumIntPixel = {.rgbtRed = 0, .rgbtGreen = 0, .rgbtBlue = 0};
    RGBTRIPLEDouble sumDoublePixelGX = {.rgbtRed = 0, .rgbtGreen = 0, .rgbtBlue = 0};
    RGBTRIPLEDouble sumDoublePixelGY = {.rgbtRed = 0, .rgbtGreen = 0, .rgbtBlue = 0};
    RGBTRIPLE returnPixel = {.rgbtRed = 0, .rgbtGreen = 0, .rgbtBlue = 0};

    numberOfPixelsSummed = 0;

    for (int subRow = row - 1, gKernelRow = 0; subRow < row + 2; subRow++, gKernelRow++)
    {
        for (int subColumn = column - 1, gKernelColumn = 0; subColumn < column + 2; subColumn++, gKernelColumn++)
        {
            //ignore pixel if outside of image
            if (subRow >= 0 && subRow < heightGlobal && subColumn >= 0 && subColumn < widthGlobal)
            {
                RGBTRIPLE currentPixel = image[subRow][subColumn];

                int gxKernelValue = gxKernel[gKernelRow][gKernelColumn];
                int gyKernelValue = gyKernel[gKernelRow][gKernelColumn];

                if (strcmp(filter, "blur") == 0)
                {
                    sumIntPixel = AddCurrentValuesToSumIntPixel(currentPixel, sumIntPixel);
                }
                else if (strcmp(filter, "edges") == 0)
                {
                    sumDoublePixelGX = GetGValuesForCurrentPixelAndAddToSum(currentPixel, gxKernelValue, sumDoublePixelGX);
                    sumDoublePixelGY = GetGValuesForCurrentPixelAndAddToSum(currentPixel, gyKernelValue, sumDoublePixelGY);
                }
                else
                {
                    // Future-proofing. The current code should never get to here.
                    printf("Filter blur or edges Not Chosen.  Please check commandline arguments.\n");
                    break;
                }
            }
        }
    }

    if (strcmp(filter, "blur") == 0)
    {
        returnPixel = AverageValuesOfSumIntPixel(sumIntPixel);
    }
    else if (strcmp(filter, "edges") == 0)
    {
        returnPixel = MergeGXAndGYPixels(sumDoublePixelGX, sumDoublePixelGY);
    }
    else
    {
        // Future-proofing. The current code should never get to here.
        printf("Filter blur or edges Not Chosen.  Please check commandline arguments.\n");
    }

    return returnPixel;
}

RGBTRIPLEInt AddCurrentValuesToSumIntPixel(RGBTRIPLE currentPixel, RGBTRIPLEInt sumIntPixel)
{
    sumIntPixel.rgbtRed += currentPixel.rgbtRed;
    sumIntPixel.rgbtGreen += currentPixel.rgbtGreen;
    sumIntPixel.rgbtBlue += currentPixel.rgbtBlue;

    numberOfPixelsSummed++;

    return sumIntPixel;
}

RGBTRIPLEDouble GetGValuesForCurrentPixelAndAddToSum(RGBTRIPLE currentPixel, int gKernelValue, RGBTRIPLEDouble sumDoublePixel)
{
    RGBTRIPLEDouble doublePixel = {.rgbtRed = 0, .rgbtGreen = 0, .rgbtBlue = 0};

    doublePixel = GetGValuesForCurrentPixel(currentPixel, gKernelValue);
    sumDoublePixel = AddGValuesToSumDoublePixel(doublePixel, sumDoublePixel);

    return sumDoublePixel;
}


RGBTRIPLEDouble GetGValuesForCurrentPixel(RGBTRIPLE currentPixel, int gKernelValue)
{
    RGBTRIPLEDouble doublePixel = {.rgbtRed = 0, .rgbtGreen = 0, .rgbtBlue = 0};

    doublePixel.rgbtRed = currentPixel.rgbtRed * gKernelValue;
    doublePixel.rgbtGreen = currentPixel.rgbtGreen * gKernelValue;
    doublePixel.rgbtBlue = currentPixel.rgbtBlue * gKernelValue;

    return doublePixel;
}

RGBTRIPLEDouble AddGValuesToSumDoublePixel(RGBTRIPLEDouble doublePixelG, RGBTRIPLEDouble sumDoublePixel)
{
    sumDoublePixel.rgbtRed += doublePixelG.rgbtRed;
    sumDoublePixel.rgbtGreen += doublePixelG.rgbtGreen;
    sumDoublePixel.rgbtBlue += doublePixelG.rgbtBlue;

    return sumDoublePixel;
}

RGBTRIPLE AverageValuesOfSumIntPixel(RGBTRIPLEInt sumIntPixel)
{
    RGBTRIPLE returnPixel = {.rgbtRed = 0, .rgbtGreen = 0, .rgbtBlue = 0};

    returnPixel.rgbtRed += GetRoundedAverage(sumIntPixel.rgbtRed, numberOfPixelsSummed);
    returnPixel.rgbtGreen += GetRoundedAverage(sumIntPixel.rgbtGreen, numberOfPixelsSummed);
    returnPixel.rgbtBlue += GetRoundedAverage(sumIntPixel.rgbtBlue, numberOfPixelsSummed);

    return returnPixel;
}

RGBTRIPLE MergeGXAndGYPixels(RGBTRIPLEDouble GX, RGBTRIPLEDouble GY)
{
    RGBTRIPLE returnPixel = {.rgbtRed = 0, .rgbtGreen = 0, .rgbtBlue = 0};

    // Perform formula to merge gX + gY - sqrt(gX^2 + gY^2)
    double finalGValueRed = round(sqrt(pow(GX.rgbtRed, 2) + pow(GY.rgbtRed, 2)));
    double finalGValueGreen = round(sqrt(pow(GX.rgbtGreen, 2) + pow(GY.rgbtGreen, 2)));
    double finalGValueBlue = round(sqrt(pow(GX.rgbtBlue, 2) + pow(GY.rgbtBlue, 2)));

    // Cap at 255 for RGB value
    returnPixel.rgbtRed = (finalGValueRed > 255) ? 255 : finalGValueRed;
    returnPixel.rgbtGreen = (finalGValueGreen > 255) ? 255 : finalGValueGreen;
    returnPixel.rgbtBlue = (finalGValueBlue > 255) ? 255 : finalGValueBlue;

    return returnPixel;
}

int GetRoundedAverage(int sum, float numberOfItems)
{
    // Function created to avoid divide by 0 issue
    int average = 0;

    if (sum == 0)
    {
        average = 0;
    }
    else
    {
        average = (int)round((float)sum / (float)numberOfItems);
    }

    average = round(average);

    return (int)average;
}

void copyRGBTRIPLEArray(RGBTRIPLE(*source)[widthGlobal], RGBTRIPLE(*copy)[widthGlobal])
{
    memcpy(copy, source, heightGlobal * widthGlobal * sizeof(RGBTRIPLE));
}