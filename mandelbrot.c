#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define ITERATIONS 100
#define IMAGE_WIDTH 800
#define IMAGE_HEIGHT 600
#define MIN_REAL -2.5
#define MAX_REAL 1.5
#define MIN_IMAGINARY -2.0
#define MAX_IMAGINARY 2.0

struct ComplexNumber {
    double real;
    double imag;
};

/*
Function that returns the number of iterations required
for the given complex number to escape the Mandelbrot set
*/
int getIterations(struct ComplexNumber c) {
    int count = 0;
    double lengthSq = 0;
    double temp;
    struct ComplexNumber z;
    z.real = 0;
    z.imag = 0;
    while ((lengthSq <= 4.0) && (count < ITERATIONS)) {
        temp = z.real * z.real - z.imag * z.imag + c.real;
        z.imag = 2 * z.real * z.imag + c.imag;
        z.real = temp;
        lengthSq = z.real * z.real + z.imag * z.imag;
        count++;
    }
    return count;
}

/*
Function that generates the Mandelbrot set using a sequential algorithm
*/
void generateSequentialMandelbrot() {
    struct ComplexNumber c;
    double scaleReal = (MAX_REAL - MIN_REAL) / IMAGE_WIDTH;
    double scaleImag = (MAX_IMAGINARY - MIN_IMAGINARY) / IMAGE_HEIGHT;

    FILE *image;
    image = fopen("Mandelbrot.ppm", "w");
    fprintf(image, "P3\n%d %d\n255\n", IMAGE_WIDTH, IMAGE_HEIGHT);

    for (int y = IMAGE_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            c.real = MIN_REAL + ((double)x * scaleReal);
            c.imag = MIN_IMAGINARY + ((double)y * scaleImag);
            int iterations = getIterations(c);
            int color = iterations;
            printf("Coloring pixel x: %d y: %d color: %d.\r", x, y, color);
            fprintf(image, "%d %d %d ", color, color, color);
        }
    }

    fclose(image);
}

/*
Function that generates the Mandelbrot set using a parallel algorithm
*/
void generateParallelMandelbrot(int rank, int size) {
    if (rank == 0) {
        double masterCommTime;
        char processorName[MPI_MAX_PROCESSOR_NAME];
        int processorNameLength;
        MPI_Get_processor_name(processorName, &processorNameLength);
        MPI_Status status;
        printf("Master Rank = %d on %s: Sending rows...\n", rank, processorName);
MPI_Barrier(MPI_COMM_WORLD);
    int rowPerProcessor = IMAGE_HEIGHT / (size - 1);
    int **matrix = malloc(sizeof(int*) * IMAGE_HEIGHT);
    for (int i = 0; i < IMAGE_HEIGHT; i++) {
        matrix[i] = malloc(sizeof(int) * IMAGE_WIDTH);
    }

    int row = 0;
    int increment = rowPerProcessor;

    for (int i = 1; i < size; i++) {
        MPI_Send(&row, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        row += increment;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Receive rows from workers and print progress
    for (int i = 1; i < size; i++) {
        int startRow = (i - 1) * rowPerProcessor;
        for (int j = startRow; j < startRow + rowPerProcessor; j++) {
            MPI_Recv(matrix[j], IMAGE_WIDTH, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            printf("Rank = %d: Received row %d from rank %d.\r", rank, j, i);
        }
    }

    // Write the image to file
    FILE *image;
    image = fopen("Mandelbrot.ppm", "w");
    fprintf(image, "P3\n%d %d\n255\n", IMAGE_WIDTH, IMAGE_HEIGHT);

    for (int y = IMAGE_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            int color = matrix[y][x];
            fprintf(image, "%d %d %d ", color, color, color);
        }
    }

    fclose(image);

    // Free dynamically allocated memory
    for (int i = 0; i < IMAGE_HEIGHT; i++) {
        free(matrix[i]);
    }
    free(matrix);

} else {
    int startRow;
    MPI_Recv(&startRow, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    int endRow = startRow + (IMAGE_HEIGHT / (size - 1));
    for (int y = startRow; y < endRow; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            struct ComplexNumber c;
            c.real = MIN_REAL + ((double)x * ((MAX_REAL - MIN_REAL) / IMAGE_WIDTH));
            c.imag = MIN_IMAGINARY + ((double)y * ((MAX_IMAGINARY - MIN_IMAGINARY) / IMAGE_HEIGHT));
            int iterations = getIterations(c);
            MPI_Send(&iterations, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }
}

MPI_Finalize();
}

int main(int argc, char** argv) {
int rank, size;
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);

if (rank == 0) {
    generateParallelMandelbrot(rank, size);
} else {
    generateParallelMandelbrot(rank, size);
}

return 0;
