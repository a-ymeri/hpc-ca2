#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define ROOT 0

int log2_(int n)
{
    int k = 0;
    while (n >>= 1)
        k++;
    return k;
}

int cmp(const void *a, const void *b)
{
    // compare 2 floating point numbers
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    return (fa > fb) - (fa < fb);
}

// function that checks if the array is sorted
int is_sorted(float *arr, int n)
{
    printf("%d",n);
    for (int i = 0; i < n - 1; i++)
        if (arr[i] > arr[i + 1])
            return 0;
    return 1;
}
// Function to merge two sorted float arrays
void merge(float *arr1, float *arr2, float *arr, int n1, int n2)
{
    int i = 0, j = 0, k = 0;
    while (i < n1 && j < n2)
    {
        if (arr1[i] < arr2[j])
            arr[k++] = arr1[i++];
        else
            arr[k++] = arr2[j++];
    }
    while (i < n1)
        arr[k++] = arr1[i++];
    while (j < n2)
        arr[k++] = arr2[j++];
}

int main(int argc, char *argv[])
{
    int rank, num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    //start timer
    double start = MPI_Wtime(), start2;

    // Read input data from file "a.dat"
 

    int n;
    float *unsorted_arr;

    if (rank == 0)
    {
        FILE *fp;
        fp = fopen("unsorted.txt", "r");
        if (fp == NULL)
        {
            printf("Error opening file\n");
            exit(1);
        }
        
        fscanf(fp, "%d", &n);
        unsorted_arr = malloc(n * sizeof(float));


        for (int i = 0; i < n; i++)
            fscanf(fp, "%f", &unsorted_arr[i]);
        
        start2 = MPI_Wtime();
        fclose(fp);
    }


    //broadcast n
    MPI_Bcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    // Allocate memory for temporary and sorted arrays
    float *global_arr = malloc(n * sizeof(float));

    // Calculate the send_count and displacements for the scatter operation
    int elements_per_proc = n / num_procs;
    int *send_count = malloc(num_procs * sizeof(int));
    int *displs = malloc(num_procs * sizeof(int));
    int remainder = n % num_procs;

    for (int i = 0; i < num_procs; i++)
    {
        send_count[i] = elements_per_proc;
        if (remainder > 0)
        {
            send_count[i]++;
            remainder--;
        }
        displs[i] = i == 0 ? 0 : displs[i - 1] + send_count[i - 1];
    }

    float *temp_arr = malloc(send_count[rank] * sizeof(float));

    // Scatter the input array to all processes
    MPI_Scatterv(unsorted_arr, send_count, displs, MPI_FLOAT, temp_arr, send_count[rank], MPI_FLOAT, ROOT, MPI_COMM_WORLD);

    // Sort the block assigned to each process
    qsort(temp_arr, send_count[rank], sizeof(float), cmp);

    // each rank checks if its sorted
    // printf("Rank %d is sorted: %d \n", rank, is_sorted(temp_arr, send_count[rank]));


    //create array from 0 to num_procs
    int *arr = malloc(num_procs * sizeof(int));
    for (int i = 0; i < num_procs; i++)
        arr[i] = i;


 // Perform iterative merging of data
    int d = 1;
    int log2_procs = log2_(num_procs);
    for (int i = 0; i < log2_procs; i++)
    {
        int partner = rank ^ d;
        if (rank < partner)
        {
            // Receive data from partner
            int recv_count;
            MPI_Recv(&recv_count, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            float *recv_arr = malloc(recv_count * sizeof(float));
            MPI_Recv(recv_arr, recv_count, MPI_FLOAT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Merge received data with local data
            merge(temp_arr, recv_arr, global_arr, send_count[rank], recv_count);
            // free(temp_arr);
            // free(recv_arr);

            // Update local data
            temp_arr = global_arr;
            send_count[rank] += recv_count;
        }
        else
        {
            // Send data to partner
            MPI_Send(&send_count[rank], 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
            MPI_Send(temp_arr, send_count[rank], MPI_FLOAT, partner, 0, MPI_COMM_WORLD);
            break;
        }
        d *= 2;
    }

    if (rank == ROOT)
    {
        printf("Rank %d is sorted: %d \n", rank, is_sorted(temp_arr, send_count[rank]));

        //end timer
        double end = MPI_Wtime();

        printf("Time taken: %f \n", end - start);
        printf("Time taken since loading data: %f \n", end - start2);
    }



    free(temp_arr);
    free(send_count);
    free(displs);

    MPI_Finalize();
    return 0;
}
