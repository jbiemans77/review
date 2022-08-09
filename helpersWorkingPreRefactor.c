#include "helpers.h"
#include <stdio.h>
#include <math.h>

// Prototypes for Functions
int GetRoundedAverage(int sum, float numberOfItems);
void copyRGBTRIPLEArray(int height, int width, RGBTRIPLE source[height][width], RGBTRIPLE copy[height][width]);

int gxKernel[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
int gyKernel[3][3] = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    // For each row
    for (int rc = 0; rc < height; rc++)
    {
        // For each pixel in that row
        for (int cc = 0; cc < width; cc++)
        {
            BYTE averageRGB = (int)round((image[rc][cc].rgbtRed + image[rc][cc].rgbtGreen + image[rc][cc].rgbtBlue)/3.0);

            image[rc][cc].rgbtRed = averageRGB;
            image[rc][cc].rgbtBlue = averageRGB;
            image[rc][cc].rgbtGreen = averageRGB;
        }
    }
    return;
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    int finalColumnToFlip = (int)round(width/2);

    // For each row
    for (int rc = 0; rc < height; rc++)
    {
        // For each pixel in that row
        for (int cc = 0; cc < finalColumnToFlip; cc++)
        {
            int mirrorLocation = (int)(width-1 - cc);

            RGBTRIPLE temp = image[rc][cc];
            image[rc][cc] = image[rc][mirrorLocation];
            image[rc][mirrorLocation] = temp;
        }
    }
    return;
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    //printf("starting Blur\n");
    RGBTRIPLE tempImage[height][width];

    copyRGBTRIPLEArray(height, width, image, tempImage);

    // For each row
    for (int rc = 0; rc < height; rc++)
    {
        // For each pixel in that row
        for (int cc = 0; cc < width; cc++)
        {
            int blurSumRed = 0;
            int blurSumBlue = 0;
            int blurSumGreen = 0;
            int blurAverageRed = 0;
            int blurAverageBlue = 0;
            int blurAverageGreen = 0;

            float numberOfPixelsSummed = 0.0;

            for(int blurRC = rc-1; blurRC < rc + 2; blurRC++)
            {
                //printf("blurRC %i\n", blurRC);
                for (int blurCC = cc-1; blurCC < cc + 2; blurCC++)
                {

                    if (blurRC >= 0 && blurRC < height && blurCC >= 0 && blurCC < width)
                    {
                        //printf("bRC %i, bCC %i, W%i, H%i ", blurRC, blurCC, width, height);

                        blurSumRed += image[blurRC][blurCC].rgbtRed;
                        blurSumGreen += image[blurRC][blurCC].rgbtGreen;
                        blurSumBlue += image[blurRC][blurCC].rgbtBlue;

                        //printf("blurSumRed: %i RED: %i\n", blurSumRed, image[rc][cc].rgbtRed);

                        numberOfPixelsSummed++;
                    }
                }
            }

            blurAverageRed = GetRoundedAverage(blurSumRed, numberOfPixelsSummed);
            blurAverageGreen = GetRoundedAverage(blurSumGreen, numberOfPixelsSummed);
            blurAverageBlue = GetRoundedAverage(blurSumBlue, numberOfPixelsSummed);

            //printf("AR %i %i AG %i %i AB %i %i NP %f W%i H%i\n", blurSumRed, blurAverageRed, blurSumGreen, blurAverageGreen, blurSumBlue, blurAverageBlue, numberOfPixelsSummed, width, height);
            tempImage[rc][cc].rgbtRed = blurAverageRed;
            tempImage[rc][cc].rgbtGreen = blurAverageGreen;
            tempImage[rc][cc].rgbtBlue = blurAverageBlue;
        }
    }

    copyRGBTRIPLEArray(height, width, tempImage, image);

    return;
}

int GetRoundedAverage(int sum, float numberOfItems)
{
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

void copyRGBTRIPLEArray(int height, int width, RGBTRIPLE source[height][width], RGBTRIPLE copy[height][width])
{
    // For each row
    for (int rc = 0; rc < height; rc++)
    {
        // For each pixel in that row
        for (int cc = 0; cc < width; cc++)
        {
            copy[rc][cc] = source[rc][cc];
        }
    }
}

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE tempImage[height][width];

    //copyRGBTRIPLEArray(height, width, image, tempImage);

    // For each row
    for (int rc = 0; rc < height; rc++)
    {
        // For each pixel in that row
        for (int cc = 0; cc < width; cc++)
        {
            double edgeSumRedGX = 0;
            double edgeSumBlueGX = 0;
            double edgeSumGreenGX = 0;
            double edgeSumRedGY = 0;
            double edgeSumBlueGY = 0;
            double edgeSumGreenGY = 0;

            double finalGValueRed = 0;
            double finalGValueGreen = 0;
            double finalGValueBlue = 0;

            float numberOfPixelsSummed = 0.0;

            //printf("%i %i ", rc, cc);
            //printf("value Red: %i ", image[rc][cc].rgbtRed);
            //printf("value Green: %i ", image[rc][cc].rgbtGreen);
            //printf("value Blue: %i \n", image[rc][cc].rgbtBlue);

            for(int edgeRC = rc-1, gRC = 0; edgeRC < rc + 2; edgeRC++, gRC++)
            {
                //printf("edgeRC %i\n", edgeRC);
                for (int edgeCC = cc-1, gCC = 0; edgeCC < cc + 2; edgeCC++, gCC++)
                {

                    if (edgeRC >= 0 && edgeRC < height && edgeCC >= 0 && edgeCC < width)
                    {
                        //printf("bRC %i, bCC %i, W%i, H%i, GRC%i, GCC%i, vGX:%i, vGY:%i. ", edgeRC, edgeCC, width, height, gRC, gCC, gxKernel[gRC][gCC], gyKernel[gRC][gCC]);

                        edgeSumRedGX += image[edgeRC][edgeCC].rgbtRed * gxKernel[gRC][gCC];
                        edgeSumGreenGX += image[edgeRC][edgeCC].rgbtGreen * gxKernel[gRC][gCC];
                        edgeSumBlueGX += image[edgeRC][edgeCC].rgbtBlue * gxKernel[gRC][gCC];

                        edgeSumRedGY += image[edgeRC][edgeCC].rgbtRed * gyKernel[gRC][gCC];
                        edgeSumGreenGY += image[edgeRC][edgeCC].rgbtGreen * gyKernel[gRC][gCC];
                        edgeSumBlueGY += image[edgeRC][edgeCC].rgbtBlue * gyKernel[gRC][gCC];

                        //printf("new value Red: %i %i Red: %i ", edgeSumRedGX, edgeSumRedGY, image[edgeRC][edgeCC].rgbtRed);
                        //printf("new value Green: %i %i Green: %i ", edgeSumGreenGX, edgeSumGreenGY, image[edgeRC][edgeCC].rgbtGreen);
                        //printf("new value Blue: %i %i Blue: %i\n", edgeSumBlueGX, edgeSumBlueGY, image[edgeRC][edgeCC].rgbtBlue);
                    }
                }
            }

            finalGValueRed = round(sqrt(pow(edgeSumRedGX,2) + pow(edgeSumRedGY,2)));
            finalGValueGreen = round(sqrt(pow(edgeSumGreenGX,2) + pow(edgeSumGreenGY,2)));
            finalGValueBlue = round(sqrt(pow(edgeSumBlueGX,2) + pow(edgeSumBlueGY,2)));

            //printf("AR %i %i %i AG %i %i %i AB %i %i %i W%i H%i\n", edgeSumRedGX, edgeSumRedGY, finalGValueRed, edgeSumGreenGX, edgeSumGreenGY, finalGValueGreen, edgeSumBlueGX, edgeSumBlueGY, finalGValueBlue, width, height);
            tempImage[rc][cc].rgbtRed = (finalGValueRed > 255) ? 255 : finalGValueRed;
            tempImage[rc][cc].rgbtGreen = (finalGValueGreen > 255) ? 255 : finalGValueGreen;
            tempImage[rc][cc].rgbtBlue = (finalGValueBlue > 255) ? 255 : finalGValueBlue;
        }
    }

    copyRGBTRIPLEArray(height, width, tempImage, image);

    return;
}
