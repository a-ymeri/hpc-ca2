#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
 
int main(int argc, char *argv[]) {
  // initialize MPI
  MPI_Init(&argc, &argv);

    //get time
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

  //read a.dat
    FILE *fp;
    fp = fopen(a_file, "r");
    int m, n;

    //first line of a.dat is n and m
    fscanf(fp, "%d %d", &m, &n);

    // second line of a.dat is the array


  int matrix_size = m * n;

    float *A = NULL;
    if (rank == 0) {
        A = (float *)malloc(matrix_size * sizeof(float));
        //read second line of a.dat
        for (int i = 0; i < matrix_size; i++) {
            fscanf(fp, "%f", &A[i]);
        }
    }



    //read b.dat
    fp = fopen(b_file, "r");
    int n2, p;

    //first line of b.dat is n and p
    fscanf(fp, "%d %d", &n2, &p);

    if (n != n2) {
        printf("Error: n != n2");
        return 1;
    }

    int matrix_size_b = n * p;


    float *B = NULL;
    // if (rank == 0) {
    B = (float *)malloc(matrix_size_b * sizeof(float));
    //read second line of a.dat
    for (int i = 0; i < matrix_size_b; i++) {
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

    for (int i = 0; i < num_procs; i++) {
        // printf("remainder: %d", remainder);
        send_counts[i] = rows_per_proc * n;
        if (i < remainder) {
            send_counts[i] += n;
        }

        if(rank == 0)
            // printf("send_counts[%d]: %d \n", i, send_counts[i]);

        displs[i] = 0;
        if (i > 0) {
            displs[i] = displs[i-1] + send_counts[i-1];
        }
    }
    
    // allocate memory for the chunk
    float *chunk = (float *)malloc(send_counts[rank] * n * sizeof(float));

    //mpi block so we can time it

    MPI_Barrier(MPI_COMM_WORLD);

    double scatter_start = MPI_Wtime();

    MPI_Scatterv(A, send_counts, displs, MPI_INT, chunk, send_counts[rank], MPI_INT, 0, MPI_COMM_WORLD);


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

    //get time

    double local_start = MPI_Wtime();


    
    
    int rows = send_counts[rank] / n;
    // printf("rows: %d \n", rows);
    int chunk_c_size = rows * p;
    float *chunk_c = (float *)malloc(chunk_c_size * sizeof(float));

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j<p; j++){
            local_sum = 0;
            //loop for summatiom
            for (int k = 0; k<n; k++){
                local_sum +=  chunk[i*n + k] * B[k*p + j];
            }
            chunk_c[i*p + j] = local_sum;
        }
    }
    
    double local_end = MPI_Wtime();

    printf("rank: %d, local time: %f \n", rank, local_end - local_start);

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
    if (rank == 0) {
        final_data = (float *)malloc(m * p * sizeof(float));
    }


    int *recv_counts = NULL;
    recv_counts = (int *)malloc(num_procs * sizeof(float));


    for (int i = 0; i < num_procs; i++) {
        recv_counts[i] = send_counts[i] / n * p;
        // if(rank == 0)
            // printf("recv_counts[%d]: %d \n", i, recv_counts[i]);
    }


    for (int i = 0; i < num_procs; i++) {
        displs[i] = 0;
        if (i > 0) {
            displs[i] = displs[i - 1] + recv_counts[i - 1];
        }
    }

    MPI_Gatherv(chunk_c, recv_counts[rank] , MPI_INT, final_data, recv_counts, displs, MPI_INT, 0, MPI_COMM_WORLD);


    //get time
    double end = MPI_Wtime();
    double time = end - start;

    //print time in seconds
    if(rank == 0){
        printf("Time including data load: %f \n", time);
        printf("Time excluding data load: %f \n", end - scatter_start);
    }

    
    // print the final array on rank a0
    if (rank == 0) {

        //read c.dat
        fp = fopen(c_file, "r");
        int m2, p2;

        //first line of c.dat is m and p
        fscanf(fp, "%d %d", &m2, &p2);

        // if (m != m2 || p != p2) {
        //     printf("Error: m != m2 || p != p2");
        //     return 1;
        // }

        int matrix_size_c = m * p;

        float *C = NULL;

        C = (float *)malloc(matrix_size_c * sizeof(float));

        //read second line of c.dat

        for (int i = 0; i < matrix_size_c; i++) {
            fscanf(fp, "%f", &C[i]);
        }

        //compare final_data and C
        for (int i = 0; i < m*p; i++) {
            // printf("final_data[%d] = %f, C[%d] = %f \n", i, final_data[i], i, C[i]);
            if (final_data[i] - C[i] > 0.0001) {
                printf("Error: final_data[%d] != C[%d]", i, i);
                printf("final_data[%d] = %f, C[%d] = %f \n", i, final_data[i], i, C[i]);
                return 1;
            }
        }

        for (int i = 0; i < m*p; i++) {
            // printf("final_data[%d] = %f \n", i, final_data[i]);
        }
    }

// clean up memory and finalize MPI
// free(chunk);
// if (rank == 0) {
// free(data);
// // free(final_data);
// // free(recv_counts);
// free(displs);
// }
MPI_Finalize();

return 0;
}