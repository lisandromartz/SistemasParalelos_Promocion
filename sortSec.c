#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#define DEBUG 0

double dwalltime();
void merge(int *, int *, long int, int *);
void mergeSort_iterative(int *, long int, int *);
void printArray(int *, long int);

// Main function to test the merge sort algorithm
int main(int argc, char *argv[])
{
    long int N;
    int EXP;
    int *arr;
    int *temp;
    double timetick;
    int check = 1;
    int i;

    if ((argc < 2) || ((EXP = atoi(argv[1])) <= 0))
    {
        printf("\nUsar: %s x\n  x: Exponente para obtener un vector de 2^(x) elementos", argv[0]);
        exit(1);
    }
    N = (long int) pow(2, EXP);

    arr = (int *) malloc(sizeof(int) * N);
    if (arr == NULL)
    {
        perror("Failed to allocate memory for arr");
        exit(EXIT_FAILURE);
    }

    temp = (int *) malloc(sizeof(int) * N);
    if (temp == NULL)
    {
        perror("Failed to allocate memory for temp");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    for (i = 0; i < N; i++)
    {
        arr[i] = rand() % 10000;
    }

#if DEBUG != 0
    printf("Given array is \n");
    printArray(arr, N);
#endif

    timetick = dwalltime();

    mergeSort_iterative(arr, N, temp);
    printf("\nTime in secs %f\n", dwalltime() - timetick);

    for (i = 0; i < N - 1; i++)
    {
        check = check && (arr[i] <= arr[i + 1]);
    }

    if (check)
    {
        printf("\nSuccess!!\n");
    }
    else
    {
        printf("\nArray sorting went wrong\n");
    }

#if DEBUG != 0
    printf("\nSorted array is \n");
    printArray(arr, N);
#endif

    free(arr);
    free(temp);
    return 0;
}

// Para calcular tiempo
double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}

// Function to merge two sorted arrays of the same size into a single sorted array
void merge(int *arr1, int *arr2, long int size, int *result)
{
    long int i = 0, j = 0, k = 0;

    // Merge the two arrays into result
    while (i < size && j < size)
    {
        if (arr1[i] <= arr2[j])
        {
            result[k++] = arr1[i++];
        }
        else
        {
            result[k++] = arr2[j++];
        }
    }

    // Copy the remaining elements of arr1, if any
    while (i < size)
    {
        result[k++] = arr1[i++];
    }

    // Copy the remaining elements of arr2, if any
    while (j < size)
    {
        result[k++] = arr2[j++];
    }
}

// Iterative merge sort function
void mergeSort_iterative(int *arr, long int n, int *temp)
{
    long int curr_size;
    long int left_start, mid, right_end;
    long int i;
    for (curr_size = 1; curr_size <= n - 1; curr_size = 2 * curr_size)
    {
        for (left_start = 0; left_start < n - 1; left_start += 2 * curr_size)
        {
            mid = left_start + curr_size - 1;
            right_end = (left_start + 2 * curr_size - 1 < n - 1) ? (left_start + 2 * curr_size - 1) : (n - 1);

            // Merge subarrays arr[left_start..mid] and arr[mid+1..right_end]
            merge(arr + left_start, arr + mid + 1, curr_size, temp + left_start);

            // Copy the merged subarray back to the original array
            for (i = left_start; i <= right_end; i++)
            {
                arr[i] = temp[i];
            }
        }
    }
}

// Function to print an array
void printArray(int *data, long int size)
{
    for (int i = 0; i < size; i++)
        printf("%d ", data[i]);
    printf("\n");
}