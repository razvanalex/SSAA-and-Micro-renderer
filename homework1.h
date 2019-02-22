#ifndef HOMEWORK_H1
#define HOMEWORK_H1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

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
    unsigned char pType[2]; // The type of image (e.g. P5)
    unsigned char maxVal;   // Max value of a pixel
    unsigned int width;     // The width of image
    unsigned int height;    // The height of image
    void* pixelMatrix;      // Address to the pixel matrix
} image;

void initialize(image *im);
void render(image *im);
void writeData(const char * fileName, image *img);

#endif /* HOMEWORK_H1 */