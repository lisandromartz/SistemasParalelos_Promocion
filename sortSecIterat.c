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
void mergeSortIterativo(int*, int);
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
/*
#if PRINT != 0
    printf("Given array is \n");
    printArray(A, N);
#endif
*/
    timetick = dwalltime();

    mergeSortIterativo(A, N);
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
/*
#if PRINT != 0
    printf("\nSorted array 'A' is \n");
    printArray(A, N);
#endif
*/
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
void mergeSortIterativo(int* data, int n)
{
    int m;
    for (m = 1; m < n; m = m * 2)
    {
        for (int i = 0; i < n - m; i += 2 * m)
        {
            int left = i;
            int medio = i + m - 1;
            int right = (i + 2 * m - 1 < n - 1) ? (i + 2 * m - 1) : (n - 1);
            merge(data, left, medio, right);
        }
    }
}
#else
// Se unen los arreglos en uno solo ordenado
void merge(int *L, int *R, int parte, int *result) {
    int i = 0, j = 0, k = 0;

    while (i < parte && j < parte) {
        if (L[i] <= R[j]) {
            result[k++] = L[i++];
        } else {
            result[k++] = R[j++];
        }
    }

    // Copia los elementos restantes de L
    while (i < parte) {
        result[k++] = L[i++];
    }

    // Copia los elementos restantes de R
    while (j < parte) {
        result[k++] = R[j++];
    }
}

// Merge sort iterativo
void mergeSortIterativo(int* arr, int N) {

    int *temp = (int*)malloc(sizeof(int)*N);
    if (temp == NULL) {
        fprintf(stderr, "Alocación de memoria fallida\n");
        exit(EXIT_FAILURE);
    }

    for (int parte = 1; parte <= N-1; parte = 2*parte) {
        for (int inicio = 0; inicio < N-1; inicio += 2*parte) {
            
            int medio = inicio + parte - 1;
            int fin = (inicio + 2*parte - 1 < N-1) ? (inicio + 2*parte - 1) : (N-1);

            // Llamada al merge enviando como parámetro punteros al inicio de las dos mitades
            merge(arr + inicio, arr + medio + 1, parte, temp + inicio);
            
            // Se copia el arreglo en temp
            for (int i = inicio; i <= fin; i++) {
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
