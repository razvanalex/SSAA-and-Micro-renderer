#include "homework1.h"

int num_threads;
int resolution;

/*
    Structure used to pass arguments to thread function
*/
typedef struct {
    int threadID;   // The id of the current thread
    image *im;      // The output image
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
    Create the matrix of pixels.
*/
int createPixelMatrix(image *img)
{   
    // For invalid type, do not read the file
    if (img == NULL || img->pType[0] != 'P')
        return 0;

    // This is a grayscale image
    img->pixelMatrix = createMatrix(img->width, img->height, 
        sizeof(grayscalePixel));

    // Allocation error
    if(!img->pixelMatrix)
        return 0;

    // Success
    return 1;
}

/*
    Initialize the image, by setting the type, the width and the height, the 
    maximum value and creating the matrix of pixels
*/
void initialize(image *im) 
{
    // Initialize image properties
    im->pType[0] = 'P';
    im->pType[1] = '5';
    im->height = resolution;
    im->width = resolution;
    im->maxVal = 255;

    // Create the matrix of pixels
    if (!createPixelMatrix(im))
        return;
}

/*
    The function used to compute the distance between a point with coordinates 
    (x, y) and the line with its equation -x + 2y = 0.
*/
double distanceFunc(double x, double y)
{
    double t = -x + 2.0 * y;
    return abs(t) / sqrt(3);
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
    int i, j, pixel;
    double x, y;

    // Get the arguments passed to the function
    image *im = ((threadFuncArgs*)args)->im;
    int threadID = ((threadFuncArgs*)args)->threadID;

    // Compute constants needed in computations
    double pixelWidth = 100.0 / im->height;  
    int numPixels = im->width * im->height;
    int start = intervalStart(numPixels, num_threads, threadID);
    int end = intervalStart(numPixels, num_threads, threadID + 1) - 1;

    grayscalePixel **matrix = (grayscalePixel**)im->pixelMatrix;
    
    for (pixel = start; pixel <= end; pixel++)
    {    
        // Transform the coordinate system (starting form bottom-left corner)
        // and compute the center of the pixel
        i = pixel / im->width;
        j = pixel % im->width;
        x = pixelWidth * (j + 0.5);
        y = pixelWidth * ((double)im->height - i - 0.5);

        // Draw the pixel accordingly
        if (floor(distanceFunc(x, y)) <= 3.0) 
        {
            // Draw black pixel
            matrix[i][j].w = 0;
        }
        else 
        {
            // Draw white pixel
            matrix[i][j].w = im->maxVal;
        }
    }

    return NULL;
}


/*
    Render the image using a parallel approach. It creates the threads and then
    runs the jobs.
*/
void render(image *im) 
{
    unsigned int i;

    // Create threads in memory and space to put the arguments
    pthread_t *thread =  (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    threadFuncArgs *threadArgs = (threadFuncArgs*)malloc(num_threads * 
                                  sizeof(threadFuncArgs));
    if (!thread || !threadArgs)
        return;

    // Start each thread to do its job
    for (i = 0; i < num_threads; i++)
    {
        threadArgs[i].im = im;
        threadArgs[i].threadID = i;
        pthread_create(&(thread[i]), NULL, threadFunc, &(threadArgs[i]));
    }

    // Wait until all threads finish ther job
    for (i = 0; i < num_threads; i++)
        pthread_join(thread[i], NULL);

    // Release unneeded memory
    free(thread);
    free(threadArgs);
}


/*
    This function writes the image to the file
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
    fprintf(outFile, "%c%c\n", img->pType[0], img->pType[1]);
    fprintf(outFile, "%d %d\n", img->width, img->height);
    fprintf(outFile, "%d\n", img->maxVal);

    grayscalePixel** matrix = (grayscalePixel**)img->pixelMatrix;

    // Write the matrix of pixels to file
    for (i = 0; i < img->height; i++) 
        for (j = 0; j < img->width; j++) 
            fwrite(&(matrix[i][j]).w, sizeof(unsigned char), 1, outFile);

    // Close the file and release memory
    fclose(outFile);
    destroyMatrix((void***)&img->pixelMatrix, img->height);
}
