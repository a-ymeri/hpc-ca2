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



    double start, start2, end, end2;
    start = MPI_Wtime();

    // Read input data from file "a.dat"

    char *a_file = argv[1];
 

    int n;
    float *unsorted_arr;

    if (rank == 0)
    {
        FILE *fp;
        fp = fopen(a_file, "r");
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

    // Allocate memory for the local array
    float *temp_arr = malloc(send_count[rank] * sizeof(float));


    start2 = MPI_Wtime();
    // Scatter the input array to all processes
    MPI_Scatterv(unsorted_arr, send_count, displs, MPI_FLOAT, temp_arr, send_count[rank], MPI_FLOAT, ROOT, MPI_COMM_WORLD);

    // Sort the block assigned to each process
    qsort(temp_arr, send_count[rank], sizeof(float), cmp);


 // Perform iterative merging of data
    int d = 1;
    int log2_procs = log2_(num_procs); // since the merges are done in log2(p) steps
    for (int i = 0; i < log2_procs; i++)
    {
        int partner = rank ^ d; // bitwise XOR to find the partner process
        if (rank < partner)
        {
            // Receive data from partner
            int recv_count;
            MPI_Recv(&recv_count, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            float *recv_arr = malloc(recv_count * sizeof(float));
            MPI_Recv(recv_arr, recv_count, MPI_FLOAT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Merge received data with local data
            merge(temp_arr, recv_arr, global_arr, send_count[rank], recv_count);

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


    end2 = MPI_Wtime();
    if (rank == ROOT)
    {

        //end timer
        double end = MPI_Wtime();

        // Write sorted data to file "b.dat"
        FILE *fp;
        fp = fopen(argv[2], "w");
        if (fp == NULL)
        {
            printf("Error opening file\n");
            exit(1);
        }


        fprintf(fp, "%d \n", n);
        for (int i = 0; i < n; i++)
            fprintf(fp, "%f ", temp_arr[i]);
        fclose(fp);

    }

    end = MPI_Wtime();

    if (rank == 0){
        printf("Time taken without io: %f", end2 - start2);
        printf("Time taken: %f \n", end - start);
    }



    free(temp_arr);
    free(send_count);
    free(displs);

    MPI_Finalize();
    return 0;
}
