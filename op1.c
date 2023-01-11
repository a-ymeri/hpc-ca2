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

    // read a.dat
    char *input = argv[1];
    char *kernel = argv[2];
    char *output_file = argv[3];
    // char* output = argv[3];
    // read a.dat

    FILE *fp;
    fp = fopen(input, "r");
    int m, n; // a dimensions, m rows, n columns, b batches

    // first line of a.dat is n and m
    int b;
    fscanf(fp, "%d %d %d", &b, &m, &n);
    // b = b;

    // second line of a.dat is the array
    float *A = NULL;
    if (rank == 0)
    {
        A = (float *)malloc(sizeof(float) * b * m * n);
        for (int i = 0; i < m * n; i++)
        {
            fscanf(fp, "%f", &A[i]);
        }
    }
    // read b.dat
    fp = fopen(kernel, "r");

    float *C = NULL;
    if (rank == 0)
    {
        C = (float *)malloc(sizeof(float) * m * n);
    }

    int p;
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

    // // floor of p-1 / 2
    // int b_lower = (p - 1) / 2;
    // // ceiling of p-1 / 2
    // int b_upper = p / 2;
    // // where to begin and end the row iterator
    // int row_start = b_lower;
    // int row_end = m - b_upper;

    // // where to begin and end the col iterator
    // int col_start = b_lower;
    // int col_end = n - b_upper;

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
        if(i != 0){
            displs[i] = displs[i-1] + send_counts[i-1];
        }

    }

    // allocate memory for the chunk
    float chunk_size = send_counts[rank]; 

    float *chunk = (float *)malloc(chunk_size * sizeof(float));

    MPI_Barrier(MPI_COMM_WORLD);


    int start1 = displs[rank];
    int end1 = displs[rank] + send_counts[rank];

    // printf("rank: %d, start: %d, end: %d \n", rank, start1, end1);
    // printf("First element: %f \n", A[start1]);
    // scatter the data to all processes
    MPI_Scatterv(A, send_counts, displs, MPI_FLOAT, chunk, chunk_size, MPI_FLOAT, 0, MPI_COMM_WORLD);



    //print chunk size per rank
    printf("Rank: %d, chunk size: %f \n", rank, chunk_size);

    // double scatter_start = MPI_Wtime();

    // if(rank == 0){
    //     //copy outer layer of A to C
    //     for (int i=0; i<m; i++) {
    //         for (int j=0; j<n; j++) {
    //             if(i < row_start || i >= row_end || j < col_start || j >= col_end) { //only cope outer layer
    //                 C[i*n+j] = A[i*n+j];
    //             }
    //         }
    // }

    // }

    // scatter the data to all processes

    // if(rank == 0){
    //     //print send_counts and displs
    //     for(int i = 0; i < num_procs; i++){
    //         printf("send_counts[%d]: %d, displs[%d]: %d \n", i, send_counts[i], i, displs[i]);
    //     }
    // }

    

    //   /*

    //     |--------------------------------------------------|
    //     |                                                  |
    //     |                                                  |
    //     |         PERFORMING THE WORK                      |
    //     |                                                  |
    //     |                                                  |
    //     |--------------------------------------------------|

    double local_start = MPI_Wtime();



    // int row_count = send_counts[rank] / n;
    // int processed_rows = row_count - b_lower - b_upper;

    // float *output = NULL;
    // output = (float *)malloc(send_counts[rank] * sizeof(float));
    // // printf("%d, %d \n", row_start, col_start);

    // int i, j, x, y;


    // if (send_counts[rank] != 0)
    // {
    //     for (i = row_start; i <= row_start + processed_rows - 1; i++)
    //     { // row
    //         for (j = col_start; j < col_end; j++)
    //         { // column
    //             output[i * n + j] = 0;
    //             for (x = 0; x < p; x++)
    //             { // kernel row
    //                 for (y = 0; y < p; y++)
    //                 { // kernel column
    //                     output[i * n + j] += chunk[(i - b_lower + x) * n + (j - b_lower + y)] * B[x * p + y];
    //                 }
    //             }
    //         }
    //     }
    // }

    // //overwrite the output onto the input chunk
    // for (int i = row_start; i <= row_start + processed_rows; i++)
    // { // row
    //     for (int j = col_start; j < col_end; j++)
    //     { // column

    //         chunk[i * n + j] = output[i * n + j];
    //     }
    // }

    // float *return_chunk = NULL;
    // return_chunk = (float *)malloc(processed_rows * n * sizeof(float));

    // //copy the inner layer of the chunk to the return chunk
    // for(int i = 0; i < processed_rows; i++){
    //     for(int j = 0; j < n; j++){
    //         return_chunk[i*n+j] = chunk[(i+row_start)*n+j];
    //     }
    // }
    // int *recv_counts = NULL;
    // int *recv_displs = NULL;

    // recv_counts = (int *)malloc(num_procs * sizeof(int));
    // recv_displs = (int *)malloc(num_procs * sizeof(int));

    // for (int i = 0; i < num_procs; i++)
    // {
    //     if (send_counts[i] == 0)
    //         recv_counts[i] = 0;
    //     else
    //     {
            
    //         int no_margin = (send_counts[i] - (b_lower + b_upper) * n); // remove top and bottom margins, this is number of rows
    //         recv_counts[i] = no_margin;
    //     }

    //     if (i == 0)
    //         recv_displs[i] = b_lower * n;
    //     else
    //         recv_displs[i] = recv_displs[i - 1] + recv_counts[i - 1];

    //     if(rank == 0){
    //         for (int i = 0; i < num_procs; i++)
    //         {
    //             printf("recv_counts[%d]: %d \n", i, recv_counts[i]);
    //             printf("recv_displs[%d]: %d \n", i, recv_displs[i]);
    //         }
    //     }
    // }

    // // gather
    // MPI_Gatherv(return_chunk, recv_counts[rank], MPI_FLOAT, C, recv_counts, recv_displs, MPI_FLOAT, 0, MPI_COMM_WORLD);


    // // if (rank == 0)
    // // {
    // //     for (int i = 0; i < m * n; i++)
    // //     {
    // //         if (i % n == 0)
    // //             printf("\n");
    // //         printf("%d ", C[i]);
    // //     }
    // // }

    // // read the output to compare
    // fp = fopen(output_file, "r");
    // float *output_array = (float *)malloc(m * n * sizeof(float));

    // if (rank == 0)
    // {

    //     int dummy;
    //     fscanf(fp, "%d %d %d", &dummy, &dummy, &dummy);
    //     for (int i = 0; i < m * n; i++)
    //     {
    //         fscanf(fp, "%f", &output_array[i]);
    //     }

    //     // compare the output
    //     int error = 0;
    //     for (int i = 0; i < m * n; i++)
    //     {
    //         if ((C[i] - output_array[i]) > 1e-6)
    //         {
    //             printf("Error at index %d, expected %f, got %f \n", i, output_array[i], C[i]);
    //             error = 1;
    //             // break;
    //         }
    //     }

    //     if (error == 0)
    //         printf("Correct output \n");
    //     else
    //         printf("Incorrect output \n");
    // }



    // //     /*

    // //     |--------------------------------------------------|
    // //     |                                                  |
    // //     |                                                  |
    // //     |         GATHERING THE WORK                       |
    // //     |                                                  |
    // //     |                                                  |
    // //     |--------------------------------------------------|
    MPI_Finalize();

    return 0;
}