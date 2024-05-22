#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#define PRINT 1
#define ADJACENT_SUBARRAYS 0

double dwalltime();
#if ADJACENT_SUBARRAYS == 1
void merge(int*, int, int, int);
#else
void merge(int*, int*, int, int*);
#endif
void mergeSort_iterative(int*, int);
void printArray(int*, int);

// Main function to test the merge sort algorithm
int main(int argc, char *argv[])
{
    int N;

    double timetick;
    int checkA = 1;
    int i;

    if ((argc < 2) || ((N = atoi(argv[1])) <= 0))
    {
        printf("\nUsar: %s n\n  n: Dimension del vector", argv[0]);
        exit(1);
    }

    int* A = (int*) malloc(sizeof(int)*N);
    srand(time(NULL));
    for (int i = 0; i < N; i++)
    {
        A[i] = rand() % 10000;
    }

#if PRINT != 0
    printf("Given array is \n");
    printArray(A, N);
#endif

    timetick = dwalltime();

    mergeSort_iterative(A, N);
    printf("\n-- Iterative merge --\n");
    printf("Time in secs %f\n", dwalltime() - timetick);

    for (i = 0; i < N - 1; i++)
    {
        checkA = checkA && (A[i] <= A[i + 1]);
    }

    if (checkA)
    {
        printf("\nArray 'A' sorting went okay\n");
    }
    else
    {
        printf("\nArray 'A' sorting went wrong\n");
    }

#if PRINT != 0
    printf("\nSorted array 'A' is \n");
    printArray(A, N);
#endif
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

#if ADJACENT_SUBARRAYS == 1
// Function to merge two subarrays data[l..m] and data[m+1..r] of array data[]
void merge(int* data, int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    // Create temporary arrays
    int* L = (int*) malloc(sizeof(int) * n1);
	int* R = (int*) malloc(sizeof(int) * n2);

    // Copy data to temporary arrays L[] and R[]
    for (i = 0; i < n1; i++)
        L[i] = data[l + i];
    for (j = 0; j < n2; j++)
        R[j] = data[m + 1 + j];

    // Merge the temporary arrays back into data[l..r]
    i = 0; // Initial index of first subarray
    j = 0; // Initial index of second subarray
    k = l; // Initial index of merged subarray
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            data[k] = L[i];
            i++;
        }
        else
        {
            data[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of L[], if any
    while (i < n1)
    {
        data[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of R[], if any
    while (j < n2)
    {
        data[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

// Iterative implementation of merge sort
void mergeSort_iterative(int* data, int n)
{
    int m;
    for (m = 1; m < n; m = m * 2)
    {
        for (int i = 0; i < n - m; i += 2 * m)
        {
            int left = i;
            int mid = i + m - 1;
            int right = (i + 2 * m - 1 < n - 1) ? (i + 2 * m - 1) : (n - 1);
            merge(data, left, mid, right);
        }
    }
}
#else
// Function to merge two sorted arrays of the same size into a single sorted array
void merge(int *arr1, int *arr2, int size, int *result) {
    int i = 0, j = 0, k = 0;

    // Merge the two arrays into result
    while (i < size && j < size) {
        if (arr1[i] <= arr2[j]) {
            result[k++] = arr1[i++];
        } else {
            result[k++] = arr2[j++];
        }
    }

    // Copy the remaining elements of arr1, if any
    while (i < size) {
        result[k++] = arr1[i++];
    }

    // Copy the remaining elements of arr2, if any
    while (j < size) {
        result[k++] = arr2[j++];
    }
}

// Iterative merge sort function
void mergeSort_iterative(int* arr, int n) {
    int *temp = (int *)malloc(n * sizeof(int));
    if (temp == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (int curr_size = 1; curr_size <= n-1; curr_size = 2*curr_size) {
        for (int left_start = 0; left_start < n-1; left_start += 2*curr_size) {
            int mid = left_start + curr_size - 1;
            int right_end = (left_start + 2*curr_size - 1 < n-1) ? (left_start + 2*curr_size - 1) : (n-1);

            // Merge subarrays arr[left_start..mid] and arr[mid+1..right_end]
            merge(arr + left_start, arr + mid + 1, curr_size, temp + left_start);
            
            // Copy the merged subarray back to the original array
            for (int i = left_start; i <= right_end; i++) {
                arr[i] = temp[i];
            }
        }
    }

    free(temp);
}
#endif

// Function to print an array
void printArray(int* data, int size)
{
    for (int i = 0; i < size; i++)
        printf("%d ", data[i]);
    printf("\n");
}