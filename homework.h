#ifndef HOMEWORK_H
#define HOMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/*
    Data type for a colored pixel (with red, green, and blue - RGB)
*/
typedef struct {
    unsigned char r, g, b;
} colorPixel;

/*
    Data type for a grayscale pixel (only with black and white)
*/
typedef struct {
    unsigned char w;
} grayscalePixel;

/*
    The image structure that stores all information needed
*/
typedef struct {
    unsigned char pType[3]; // The type of image as string ( + '\0' at end)
    unsigned char maxVal;   // Max value of a pixel
    unsigned int width;     // The width of image
    unsigned int height;    // The height of image
    void* pixelMatrix;      // Address to the pixel matrix
} image;

void readInput(const char * fileName, image *img);

void writeData(const char * fileName, image *img);

void resize(image *in, image * out);

#endif /* HOMEWORK_H */