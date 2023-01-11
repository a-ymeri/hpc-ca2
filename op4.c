#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define ROOT 0

int cmp(const void *a, const void *b)
{
    float diff = (*(float *)a - *(float *)b);
    if(diff > 0)
        return 1;
    else if(diff < 0)
        return -1;
    else
        return 0;
    // return (*(float *)a - *(float *)b);
}

// Function to merge two sorted arrays
void merge(float *a, float *b, float *c, int m, int n)
{
    int i = 0, j = 0, k = 0;

    while (i < m && j < n)
    {
        if (a[i] < b[j])
            c[k++] = a[i++];
        else
            c[k++] = b[j++];
    }

    // Append remaining elements from input arrays
    while (i < m)
        c[k++] = a[i++];
    while (j < n)
        c[k++] = b[j++];
}

int main(int argc, char *argv[])
{
    int my_rank, num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Read input data from file "a.dat"
    FILE *fp;
    fp = fopen("unsorted.txt", "r");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    int n;
    fscanf(fp, "%d", &n);
    float *arr = malloc(n * sizeof(float));
    printf("n = %d", n);
    for (int i = 0; i < n; i++)
        fscanf(fp, "%f", &arr[i]);
    fclose(fp);

    float *temp_arr = malloc(n * sizeof(float));
    float *recv_buf = malloc(n * sizeof(float));
    float *sorted_arr = malloc(n * sizeof(float));
    float *global_arr = malloc(n * sizeof(float));

   // Divide the array into equal-sized blocks and assign them to processes
    int block_size = n / num_procs;
    int start = my_rank * block_size;
    int end = start + block_size - 1;
    if (my_rank == num_procs - 1)
        end = n - 1;

    // Sort the block assigned to each process
    for (int i = start; i <= end; i++)
        temp_arr[i - start] = arr[i];
    qsort(temp_arr, end - start + 1, sizeof(float), cmp);

    // Merge the sorted blocks using a recursive doubling algorithm
    for (int step = 1; step < num_procs; step *= 2)
    {
        int partner = my_rank ^ step;
        if (my_rank < partner)
        {
           MPI_Sendrecv(temp_arr, end - start + 1, MPI_FLOAT, partner, 0, recv_buf, n, MPI_FLOAT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            merge(temp_arr, recv_buf, sorted_arr, end - start + 1, n);
            memcpy(temp_arr, sorted_arr, (end - start + 1 + n) * sizeof(float));
        }
    }

    // Gather the sorted blocks on the root process
    MPI_Gather(temp_arr, end - start + 1, MPI_FLOAT, global_arr, end - start + 1, MPI_FLOAT, ROOT, MPI_COMM_WORLD);

    // Print the sorted array on the root process
    // if (my_rank == ROOT)
    // {
    //     //compare with sorted.txt
    //     FILE *fp;
    //     fp = fopen("sorted.txt", "r");
    //     if (fp == NULL)
    //     {
    //         printf("Error opening file\n");
    //         exit(1);
    //     }

    //     int n;
    //     fscanf(fp, "%d", &n);
    //     float *arr = malloc(n * sizeof(float));
    //     for (int i = 0; i < n; i++)
    //         fscanf(fp, "%f", &arr[i]);
    //     fclose(fp);

    //     int flag = 0;
    //     for (int i = 0; i < n; i++)
    //     {

    //         if (global_arr[i] != arr[i])
    //         {
    //             printf("\nSorted array is incorrect at %d\n, %f %f", i, global_arr[i], arr[i]);
    //             flag += 1;
    //             break;
    //         }
    //     }
    //     if (flag)
    //         printf("Sorted array is incorrect %d\n", flag);
    //     else
    //         printf("Sorted array is correct\n");
    // }

    if(my_rank == ROOT)
    {
        for (int i = 1; i < n; i++){
            float diff = global_arr[i] - arr[i];
            if(diff > 0.0001 || diff < -0.0001)
            {
                printf("Sorted array is incorrect at %d\n, %f %f", i, global_arr[i], arr[i]);
                break;
            }
            else
                printf("Sorted array is correct\n");

        }
        printf("\n");
    }
    MPI_Finalize();
    return 0;
}



