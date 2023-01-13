#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
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

    // read the file name from the command line
    char *a_file = argv[1];
    char *b_file = argv[2];
    char *c_file = argv[3];

    // read a.dat
    FILE *fp;
    fp = fopen(a_file, "r");
    int m, n;

    // first line of a.dat is n and m
    fscanf(fp, "%d %d", &m, &n);

    // second line of a.dat is the array
    int matrix_size = m * n;

    float *A = NULL;
    if (rank == 0)
    {
        A = (float *)malloc(matrix_size * sizeof(float));

        // read second line of a.dat
        for (int i = 0; i < matrix_size; i++)
        {
            fscanf(fp, "%f", &A[i]);
        }
    }

    // read b.dat
    fp = fopen(b_file, "r");
    int n2, p;

    // first line of b.dat is n and p
    fscanf(fp, "%d %d", &n2, &p);

    if (n != n2)
    {
        printf("Error: n != n2");
        return 1;
    }

    int matrix_size_b = n * p;

    float *B = NULL;
    // if (rank == 0) {
    B = (float *)malloc(matrix_size_b * sizeof(float));
    // read second line of a.dat
    for (int i = 0; i < matrix_size_b; i++)
    {
        fscanf(fp, "%f", &B[i]);
    }

    // }

    /*

                                           |--------------------------------------------------|
                                           |                                                  |
                                           |                                                  |
                                           |         SPLITTING THE WORK                       |
                                           |                                                  |
                                           |                                                  |
                                           |--------------------------------------------------|

    */
    int rows_per_proc = m / num_procs;
    int remainder = m % num_procs;

    int *send_counts = NULL;
    send_counts = (int *)malloc(num_procs * sizeof(float));

    int *displs = NULL;
    displs = (int *)malloc(num_procs * sizeof(float));

    for (int i = 0; i < num_procs; i++)
    {
        send_counts[i] = rows_per_proc * n;
        if (i < remainder)
        {
            send_counts[i] += n;
        }

        displs[i] = 0;
        if (i > 0)
        {
            displs[i] = displs[i - 1] + send_counts[i - 1];
        }
    }

    // allocate memory for the chunk
    float *chunk = (float *)malloc(send_counts[rank] * sizeof(float));

    double work_start = MPI_Wtime();

    MPI_Scatterv(A, send_counts, displs, MPI_FLOAT, chunk, send_counts[rank], MPI_FLOAT, 0, MPI_COMM_WORLD);

    /*

      |--------------------------------------------------|
      |                                                  |
      |                                                  |
      |         PERFORMING THE WORK                      |
      |                                                  |
      |                                                  |
      |--------------------------------------------------|

    */
    float local_sum;


    int rows = send_counts[rank] / n;
    // printf("rows: %d \n", rows);
    int chunk_c_size = rows * p;
    float *chunk_c = (float *)malloc(chunk_c_size * sizeof(float));

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < p; j++)
        {
            local_sum = 0;
            // loop for summatiom
            for (int k = 0; k < n; k++)
            {
                local_sum += chunk[i * n + k] * B[k * p + j];
            }
            chunk_c[i * p + j] = local_sum;
        }
    }

    double local_end = MPI_Wtime();

    /*

    |--------------------------------------------------|
    |                                                  |
    |                                                  |
    |         GATHERING THE WORK                       |
    |                                                  |
    |                                                  |
    |--------------------------------------------------|

    */

    // // gather the array back to rank 0
    float *final_data = NULL;
    if (rank == 0)
    {
        final_data = (float *)malloc(m * p * sizeof(float));
    }

    int *recv_counts = NULL;
    recv_counts = (int *)malloc(num_procs * sizeof(float));

    for (int i = 0; i < num_procs; i++)
    {
        recv_counts[i] = send_counts[i] / n * p;
    }

    for (int i = 0; i < num_procs; i++)
    {
        displs[i] = 0;
        if (i > 0)
        {
            displs[i] = displs[i - 1] + recv_counts[i - 1];
        }
    }

    MPI_Gatherv(chunk_c, recv_counts[rank], MPI_INT, final_data, recv_counts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    double work_end = MPI_Wtime();

    if (rank == 0)
    {
        // print the result to c_file
        fp = fopen(c_file, "w");
        fprintf(fp, "%d %d \n", m, p);
        for (int i = 0; i < m * p; i++)
        {
            fprintf(fp, "%f ", final_data[i]);
        }

        fclose(fp);
    }

    double end = MPI_Wtime();

    if (rank == 0)
    {
        printf("total time: %f \n", end - start);
        printf("work time: %f \n", work_end - work_start);
    }

    MPI_Finalize();

    return 0;
}