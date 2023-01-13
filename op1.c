#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

int main(int argc, char **argv)
{

    // initialize MPI
    MPI_Init(&argc, &argv);

    // get time
    double start = MPI_Wtime();

    // get the number of processes
    int num_procs;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // get the rank of the current process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // get sys args
    char *input = argv[1];
    char *kernel = argv[2];
    char *output_file = argv[3];

    // read a.dat
    FILE *fp;
    int m, n, b;

    float *A = NULL;
    fp = fopen(input, "r");
    fscanf(fp, "%d %d %d", &b, &m, &n);


    //root rank reads the matrix
    if (rank == 0)
    {

        A = (float *)malloc(sizeof(float) * b * m * n);
        for (int i = 0; i < b * m * n; i++)
        {
            fscanf(fp, "%f", &A[i]);
        }
    }


    float *C = NULL;
    if (rank == 0)
    {
        C = (float *)malloc(sizeof(float) * b * m * n);
    }

    int p;

    fp = fopen(kernel, "r");

    // first line of b.dat is p and p. Only need one of them
    fscanf(fp, "%d %d", &p, &p);

    float *B = NULL;
    B = (float *)malloc(sizeof(float) * p * p);
    for (int i = 0; i < p * p; i++)
    {
        fscanf(fp, "%f", &B[i]);
    }

    //   /*

    //                                          |--------------------------------------------------|
    //                                          |                                                  |
    //                                          |                                                  |
    //                                          |         SPLITTING THE WORK                       |
    //                                          |                                                  |
    //                                          |                                                  |
    //                                          |--------------------------------------------------|

    //   */

    double start_of_work = MPI_Wtime();
    // floor of p-1 / 2
    int b_lower = (p - 1) / 2;
    // ceiling of p-1 / 2
    int b_upper = p / 2;
    // where to begin and end the row iterator
    int row_start = b_lower;
    int row_end = m - b_upper;

    // where to begin and end the col iterator
    int col_start = b_lower;
    int col_end = n - b_upper;

    int matrices_per_process = b / num_procs;
    int remainder = b % num_procs;

    int *send_counts = NULL;
    int *displs = NULL;

    send_counts = (int *)malloc(num_procs * sizeof(int));
    displs = (int *)malloc(num_procs * sizeof(int));

    for (int i = 0; i < num_procs; i++)
    {
        send_counts[i] = matrices_per_process * m * n;
        if (i < remainder)
        {
            send_counts[i] += m * n;
        }

        displs[i] = 0;
        if (i != 0)
        {
            displs[i] = displs[i - 1] + send_counts[i - 1];
        }
    }

    // allocate memory for the chunk
    int chunk_size = send_counts[rank];

    float *chunk = (float *)malloc(chunk_size * sizeof(float));

    MPI_Scatterv(A, send_counts, displs, MPI_FLOAT, chunk, chunk_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // copy chunk to output
    float *output = (float *)malloc(chunk_size * sizeof(float));
    for (int i = 0; i < chunk_size; i++)
    {
        output[i] = chunk[i];
    }


    // split chunk into b matrices
    for (int i = 0; i < (chunk_size / (m * n)); i++)
    {

        float *matrix = (float *)malloc(m * n * sizeof(float));
        for (int j = 0; j < m * n; j++)
        {
            matrix[j] = chunk[i * m * n + j];
        }
        // apply convolution
        for (int j = row_start; j < row_end; j++)
        {
            for (int k = col_start; k < col_end; k++)
            {
                float sum = 0;
                for (int l = 0; l < p; l++)
                {
                    for (int m = 0; m < p; m++)
                    {
                        sum += matrix[(j - b_lower + l) * n + (k - b_lower + m)] * B[l * p + m];
                    }
                }
                output[i * m * n + j * n + k] = sum;
            }
        }
    }

    //     |--------------------------------------------------|
    //     |                                                  |
    //     |                                                  |
    //     |         GATHERING THE WORK                       |
    //     |                                                  |
    //     |                                                  |
    //     |--------------------------------------------------|

    // gather the data to all processes
    MPI_Gatherv(output, chunk_size, MPI_FLOAT, C, send_counts, displs, MPI_FLOAT, 0, MPI_COMM_WORLD);




    // time end of work
    double end_of_work = MPI_Wtime();
    if (rank == 0)
    {
        // write output to file
        fp = fopen(output_file, "w");

        fprintf(fp, "%d %d %d \n", b, m, n);
        for (int i = 0; i < b * m * n; i++)
        {
            fprintf(fp, "%f ", C[i]);
        }
    }

    double end = MPI_Wtime();

    if (rank == 0)
    {
        printf("Time taken for work: %f \n", end_of_work - start_of_work);
        printf("Time taken in total: %f \n", end - start);
    }

    MPI_Finalize();
    return 0;
}