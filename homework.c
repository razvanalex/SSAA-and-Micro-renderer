#include "homework.h"

int num_threads;
int resize_factor;

/*
    Structure used to pass arguments to a thread function
*/
typedef struct {
    int threadID;   // The id of current thread
    image *in;      // The input image
    image *out;     // The output image
} threadFuncArgs;


/*
    Allocate memory for a matrix of height x width size, having each element
    of sizeT. This is a generic implementation for any type of matrix.
*/
void** createMatrix(unsigned int width, unsigned int height, unsigned int sizeT)
{
    unsigned int i, j;
    void** matrix;

    matrix = (void**)malloc(height * sizeof(void*));
    if (matrix == NULL)
        return NULL;

    for (i = 0; i < height; i++) 
    {
        matrix[i] = malloc(width * sizeT);

        // Error while allocation memory
        if (matrix[i] == NULL)
        {
            for (j = 0; j < i; j++)
                free(matrix[j]);
            return NULL;
        }
    }

    return matrix;
}


/*
    Release the memory of a generic matrix.
*/
void destroyMatrix(void*** matrix, unsigned int height)
{  
    unsigned int i;

    for (i = 0; i < height; i++)
        if ((*matrix)[i] != NULL)
            free((*matrix)[i]);
 
    free(*matrix);
    *matrix = NULL;
}


/*
    Create the pixel matrix, depending on the type of the image (grayscale of 
    colored).
*/
int createPixelMatrix(image *img)
{   
    // For invalid type, do not create a pixel matrix
    if (img == NULL || img->pType[0] != 'P')
        return 0;

    // Identify the type of image
    if (img->pType[1] == '5') 
    {
        // This is a grayscale image
        img->pixelMatrix = createMatrix(img->width, img->height, 
            sizeof(grayscalePixel));
    } 
    else if (img->pType[1] == '6')
    {
        // This is a colored image
        img->pixelMatrix = createMatrix(img->width, img->height, 
            sizeof(colorPixel));
    }

    // Allocation error
    if(!img->pixelMatrix)
        return 0;

    // Success
    return 1;
}


/*
    Read the image form a given file.
*/
void readInput(const char * fileName, image *img) 
{
    unsigned int i, j;
    FILE* inFile;

    // Open the file in binary reading mode
    inFile = fopen(fileName, "rb");
    if (!inFile)
        return;

    // Clear garbage data of image
    memset(img, 0, sizeof(image));    

    // Read the type from the image file and skip the '\n' character
    fread(img->pType, sizeof(unsigned char), 2, inFile);
    fseek(inFile, 1, SEEK_CUR);

    // Read the width and the height of the image, along with maximul value
    fscanf(inFile, "%d %d\n", &img->width, &img->height);
    fscanf(inFile, "%hhu\n", &img->maxVal);
    
    // Create the matrix of pixels
    if (!createPixelMatrix(img))
        return;

    // Read the matrix of pixels
    if (img->pType[1] == '5') 
    {
        grayscalePixel** matrix = (grayscalePixel**)img->pixelMatrix;
        for (i = 0; i < img->height; i++) 
            for (j = 0; j < img->width; j++)         
                fread(&(matrix[i][j]).w, sizeof(unsigned char), 1, inFile);
    } 
    else if (img->pType[1] == '6') 
    {
        colorPixel** matrix = (colorPixel**)img->pixelMatrix;
        for (i = 0; i < img->height; i++) 
            for (j = 0; j < img->width; j++) 
                fread(&(matrix[i][j]).r, sizeof(unsigned char), 3, inFile);
    }

    // Close the file
    fclose(inFile);
}


/*
    Write the image to file.
*/
void writeData(const char * fileName, image *img) 
{
    unsigned int i, j;
    FILE* outFile;

    // Open the file in binary writing mode
    outFile = fopen(fileName, "wb");
    if (!outFile)
        return;
    
    // Write image metadata to file
    fprintf(outFile, "%c%c\n", (img->pType)[0], (img->pType)[1]);
    fprintf(outFile, "%d %d\n", img->width, img->height);
    fprintf(outFile, "%d\n", img->maxVal);

    // Read the matrix of pixels
    if (img->pType[1] == '5')
    {
        grayscalePixel** matrix = (grayscalePixel**)img->pixelMatrix;
        for (i = 0; i < img->height; i++) 
            for (j = 0; j < img->width; j++) 
                fwrite(&matrix[i][j].w, sizeof(unsigned char), 1, outFile);
    } 
    else if (img->pType[1] == '6')
    {
        colorPixel** matrix = (colorPixel**)img->pixelMatrix;
        for (i = 0; i < img->height; i++) 
            for (j = 0; j < img->width; j++) 
                fwrite(&matrix[i][j].r, sizeof(unsigned char), 3, outFile);
    }

    // Close the file and release memory for output image
    fclose(outFile);
    destroyMatrix((void***)&img->pixelMatrix, img->height);
}


/*
    Compute the color of the pixel from output image, to reduce the 
    aliasing effect, by reducing the size of the original image.
    This function is for a grayscale image. For an even resize factor compute 
    the arithmetic average and for a resize factor of 3, apply the Gaussian
    Kernel. 
*/
void computeGrayscale(image *out, unsigned int x, unsigned int y, image *img)
{
    // Declare and initialize variables
    unsigned int i, j;
    
    unsigned int startX = x * resize_factor;
    unsigned int endX = (x + 1) * resize_factor - 1;
    unsigned int startY = y * resize_factor;
    unsigned int endY = (y + 1) * resize_factor - 1;
    
    unsigned int sumW = 0;
    unsigned int numPixels = resize_factor * resize_factor;

    unsigned char gaussianKernel[3][3] = {
        { 1, 2, 1 }, 
        { 2, 4, 2 }, 
        { 1, 2, 1 } 
    };

    grayscalePixel** matrix = (grayscalePixel**)img->pixelMatrix;
    grayscalePixel** outMatrix = (grayscalePixel**)out->pixelMatrix;

    // Compute new colors of pixels
    if (resize_factor % 2 == 0) 
    {
        // For an even resize_factor value
        for (i = startY; i <= endY; i++) 
            for (j = startX; j <= endX; j++) 
                sumW += matrix[i][j].w;
        
        outMatrix[y][x].w = sumW / numPixels;
    }
    else if (resize_factor == 3)
    {
        // For a resize_factor value equal to 3
        for (i = startY; i <= endY; i++)
            for (j = startX; j <= endX; j++) 
                sumW += matrix[i][j].w * gaussianKernel[i - startY][j - startX];

        outMatrix[y][x].w = sumW / 16;
    }
}


/*
    Compute the color of the pixel from output image, to reduce the 
    aliasing effect, by reducing the size of the original image.
    This function is for a colored image. For an even resize factor compute 
    the arithmetic average and for a resize factor of 3, apply the Gaussian
    Kernel. 
*/
void computeColor(image *out, unsigned int x, unsigned int y, image *img)
{
    // Declare and initialize variables
    unsigned int i, j;

    unsigned int startX = x * resize_factor;
    unsigned int endX = startX + resize_factor - 1;
    unsigned int startY = y * resize_factor;
    unsigned int endY = startY + resize_factor - 1;
    
    unsigned int sumR = 0, sumG = 0, sumB = 0;
    unsigned int numPixels = resize_factor * resize_factor;

    unsigned char gaussianKernel[3][3] = {
        { 1, 2, 1 }, 
        { 2, 4, 2 }, 
        { 1, 2, 1 } 
    };

    colorPixel** matrix = (colorPixel**)img->pixelMatrix;
    colorPixel** outMatrix = (colorPixel**)out->pixelMatrix;

    // Compute new colors of pixels
    if (resize_factor % 2 == 0) 
    {
        // For an even resize_factor value
        for (i = startY; i <= endY; i++) 
            for (j = startX; j <= endX; j++) 
            {
                sumR += matrix[i][j].r;
                sumG += matrix[i][j].g;
                sumB += matrix[i][j].b;
            }

        outMatrix[y][x].r = sumR / numPixels;
        outMatrix[y][x].g = sumG / numPixels;
        outMatrix[y][x].b = sumB / numPixels;
    }
    else if (resize_factor == 3)
    {
        // For a resize_factor value equal to 3
        for (i = startY; i <= endY; i++) 
            for (j = startX; j <= endX; j++) 
            {
                sumR += matrix[i][j].r * gaussianKernel[i - startY][j - startX];
                sumG += matrix[i][j].g * gaussianKernel[i - startY][j - startX];
                sumB += matrix[i][j].b * gaussianKernel[i - startY][j - startX];
            }
     
        outMatrix[y][x].r = sumR / 16;
        outMatrix[y][x].g = sumG / 16;
        outMatrix[y][x].b = sumB / 16;
    }
}


/*
    Compute the start of a group from P groups, and a total of N elements,
    being known the id of the group (that is Tid). The groups have sizes
    almost equal (maximum difference between any two groups is 1). 
    This is ideal to divide elements of an array, in a fairly manner, to each 
    thread.
*/
int intervalStart(int N, int P, int Tid)
{
    int size = N / P;
    int remainder = N % P;

    return Tid > remainder ? remainder + size * Tid : (size + 1) * Tid;
}


/*
    The function executed by each thread. This does a parallel algorithm to 
    create the new image, by giving linearizing the matrix and giving to each
    thread certain amount of pixels, thus for each pixel the calculations are 
    independent.
*/
void* threadFunc(void* args)
{
    unsigned int i, x, y;
    
    // Get the function arguments form the structure
    int threadID = ((threadFuncArgs*)args)->threadID;
    image *out = ((threadFuncArgs*)args)->out;
    image *in = ((threadFuncArgs*)args)->in;

    // Compute the number of pixels from the image
    int numPixels = out->width * out->height;

    // Compute the start and the end for pixels, for current thread
    int start = intervalStart(numPixels, num_threads, threadID);
    int end = intervalStart(numPixels, num_threads, threadID + 1) - 1;

    // For each pixel, compute the valuea from the original image
    for (i = start; i <= end; i++)
    {
        x = i % out->width;
        y = i / out->width; 

        if (out->pType[1] == '5') 
            computeGrayscale(out, x, y, in);
        else if (out->pType[1] == '6')  
            computeColor(out, x, y, in);
    }

    return NULL;
}


/*
    Resize the original image to reduce the aliasing effect. This function
    initializes the metadate of the output image, creates the pixel matrix,
    computes the new colors and release memory for unnecessary variables in the
    future. The computation of the new pixels is done in parallel.
*/
void resize(image *in, image *out) 
{ 
    unsigned int i;

    // Copy the type and max value, and compute the new dimensions
    memcpy(out->pType, in->pType, sizeof(in->pType));
    out->width = in->width / resize_factor;
    out->height = in->height / resize_factor;
    out->maxVal = in->maxVal;

    // Create the pixel matrix of the output
    if (!createPixelMatrix(out))
        return;

    // Create an array of threads, along with their arguments
    pthread_t *thread =  (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    threadFuncArgs *threadArgs = (threadFuncArgs*)malloc(num_threads * 
                                  sizeof(threadFuncArgs));
    if (!thread || !threadArgs)
        return;

    // Start each thread to execute
    for (i = 0; i < num_threads; i++)
    {
        threadArgs[i].in = in;
        threadArgs[i].out = out;
        threadArgs[i].threadID = i;
        pthread_create(&(thread[i]), NULL, threadFunc, &(threadArgs[i]));
    }

    // Wait all threads to finish their jobs
    for (i = 0; i < num_threads; i++)
        pthread_join(thread[i], NULL);

    // Release memory to avoid memory leaks
    free(thread);
    free(threadArgs);
    destroyMatrix((void***)&in->pixelMatrix, in->height);
}
