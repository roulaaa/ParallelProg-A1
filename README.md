# CSC447
Assignment 1 Code

Assignment1MPI - Question 3

This code generates the Mandelbrot set using a parallel algorithm implemented with MPI. The sequential algorithm is also implemented for comparison purposes.

The program first defines some constants such as the number of iterations to determine whether a complex number is inside or outside the Mandelbrot set, the size of the image to be generated, and the boundaries of the complex plane to be explored.

The ComplexNumber structure defines a complex number with real and imaginary parts.

The getIterations function takes a complex number as input and returns the number of iterations required for the given complex number to escape the Mandelbrot set.

The generateSequentialMandelbrot function generates the Mandelbrot set using a sequential algorithm. It first calculates the scaling factors for the real and imaginary axes of the complex plane. It then creates an image file and writes the header. Finally, it loops over each pixel in the image and calculates the corresponding complex number. It then calls getIterations to determine the number of iterations required for the complex number to escape the Mandelbrot set and uses this value to generate the color for the pixel.

The generateParallelMandelbrot function generates the Mandelbrot set using a parallel algorithm. It first checks whether the current process is the master process (rank 0) or a worker process. If the process is the master process, it calculates the number of rows that each worker process will be responsible for and sends the starting row index to each worker process. It then receives the results from the worker processes and writes them to an image file. If the process is a worker process, it receives the starting row index from the master process and loops over the corresponding rows. It calculates the complex number for each pixel in each row, calls getIterations to determine the number of iterations required for the complex number to escape the Mandelbrot set, and sends this value back to the master process.

The main function initializes MPI and calls either generateSequentialMandelbrot or generateParallelMandelbrot depending on the number of processes specified on the command line. It then finalizes MPI and exits.
