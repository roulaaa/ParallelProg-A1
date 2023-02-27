#include <stdio.h>

#include <stdlib.h>

#include <math.h>

#include <string.h>

#include <mpi.h>

#define WIDTH 800
#define HEIGHT 600
#define MAX_ITERS 1000

// Define a complex number struct with real and imaginary parts
struct complex {
  double real;
  double imag;
};

int main() {
  // Initialize MPI
  int rank, size;
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, & rank);
  MPI_Comm_size(MPI_COMM_WORLD, & size);

  // Compute the number of rows to process in each rank
  int chunk = HEIGHT / size;

  // Allocate memory for the Mandelbrot set data
  int * mandelbrot_set_data = malloc(WIDTH * HEIGHT * sizeof(int));
  memset(mandelbrot_set_data, 0, WIDTH * HEIGHT * sizeof(int));

  // Record the start time of the computation
  double start_t = MPI_Wtime();

  // Compute the Mandelbrot set data for each rank's rows
  int x, y;
  for (y = rank * chunk; y < (rank + 1) * chunk; y++) {
    for (x = 0; x < WIDTH; x++) {
      double real = -2.0 + 3.0 * (double) x / (double) WIDTH;
      double imag = -1.5 + 3.0 * (double) y / (double) HEIGHT;
      // Create a complex number from the computed real and imaginary parts
      struct complex c = {
        real,
        imag
      };

      // Compute the number of iterations for the complex number to escape the Mandelbrot set
      int value = mandelbrotSet(c);

      // Save the number of iterations to the data array
      mandelbrot_set_data[y * WIDTH + x] = value;
    }

  }

  // Gather the Mandelbrot set data from each rank into the root rank's data array
  MPI_Gather(mandelbrot_set_data + rank * chunk * WIDTH, chunk * WIDTH, MPI_INT,
    mandelbrot_set_data, chunk * WIDTH, MPI_INT, 0, MPI_COMM_WORLD);

  // Measure end time
  double end_t = MPI_Wtime();

  // Root rank writes PPM image and prints execution time
  if (rank == 0) {
    printf("Execution time: %.3f seconds\n", end_t - start_t);

    // Write PPM image
    char * filename = "mandelbrot.ppm";
    write_ppm(filename, mandelbrot_set_data, WIDTH, HEIGHT);
    printf("Image saved to %s\n", filename);
  }

  // Free allocated memory
  free(mandelbrot_set_data);

  // Finalize MPI environment
  MPI_Finalize();
  return 0;
}

// Compute the number of iterations it takes for a complex number to escape the
// Mandelbrot set
int mandelbrotSet(struct complex c) {
  struct complex z = {
    0.0,
    0.0
  };
  int i;
  for (i = 0; i < MAX_ITERS; i++) {
    // Compute the next value of z
    double real_temp = z.real * z.real - z.imag * z.imag + c.real;
    double imag_temp = 2.0 * z.real * z.imag + c.imag;
    z.real = real_temp;
    z.imag = imag_temp;
    // If z has escaped the Mandelbrot set, stop iterating
    if (z.real * z.real + z.imag * z.imag > 4.0) {
      break;
    }
  }
  return i;
}

// Write the Mandelbrot set data to a PPM file
void write_ppm(char * filename, int * data, int width, int height) {
  FILE * fp = fopen(filename, "wb");
  fprintf(fp, "P6\n%d %d\n255\n", width, height);
  int x, y;
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      int value = data[y * width + x];
      unsigned char r, g, b;
      if (value == MAX_ITERS) {
        r = 0;
        g = 0;
        b = 0;
      } else {
        // Compute color values based on the number of iterations
        r = (value * 7) % 256;
        g = (value * 5) % 256;
        b = (value * 3) % 256;
      }
      // Write the color values to the file
      fputc(r, fp);
      fputc(g, fp);
      fputc(b, fp);
    }
  }
  fclose(fp);
}



// The program computes the Mandelbrot set for a given range of complex numbers using the MPI library for parallelization.
//then its executed by multiple processes from MPI, where each process computes a part. The  result is gathered in the master process, which writes the result to a PPM image file.
