#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#define DEBUG 0

// Cantidad de elementos del vector
long int N;

// N = 2^(EXP)
int EXP = 10;

// Cantidad de hilos
int NUM_THREADS;

// Vector a ordenar
int *vector;

int **sorted_arrs;
int **temp_arrs;

// Variables de sincronizacion
pthread_barrier_t *barriers;

double dwalltime();
void merge(int *, int *, long int, int *);
void mergeSort_iterative(int *, long int, int *);
void printArray(int *, long int);

void *funcion(void *arg)
{
    int id = *(int *)arg;
#if DEBUG != 0
    printf("Thread id: %d\n", id);
#endif

    int i;
    int merge_threads = NUM_THREADS;
    long int slice = N / NUM_THREADS;
    long int begin = id * slice;

    // Copy my respective part of the array
    long int max_size = N / (1 << (int)ceil(log2(id + 1)));
    for (i = 0; i < slice; i++)
    {
        sorted_arrs[id][i] = vector[begin + i];
    }

    // Order my part of the array
    mergeSort_iterative(sorted_arrs[id], slice, temp_arrs[id]);

    int barrier_select = id % (merge_threads / 2);
    pthread_barrier_wait(&barriers[barrier_select]);

    merge_threads /= 2;
    while (id < merge_threads && merge_threads > 1)
    {
        // Merge my ordered part with an ordered 'right' part
        merge(sorted_arrs[id], sorted_arrs[id + merge_threads], slice, temp_arrs[id]);

        slice *= 2;
        for (i = 0; i < slice; i++)
        {
            sorted_arrs[id][i] = temp_arrs[id][i];
        }

        barrier_select = (NUM_THREADS - merge_threads) + id % (merge_threads / 2);
        pthread_barrier_wait(&barriers[barrier_select]);
        merge_threads /= 2;
    }

    if (id == 0)
    {
        merge(sorted_arrs[id], sorted_arrs[id + merge_threads], slice, temp_arrs[id]);
        slice *= 2;
        for (int i = 0; i < slice; i++)
        {
            vector[begin + i] = temp_arrs[id][i];
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    long int i;
    int cant_barrs;
    long int max_size;
    double timetick;
    int check = 1;

    // Controla los argumentos al programa
    if ((argc != 3) || ((EXP = atoi(argv[1])) <= 0) || ((NUM_THREADS = atoi(argv[2])) <= 0))
    {
        printf("\nUsar: %s x t\n    x: Exponente para obtener un vector de 2^(x) elementos\n   t: Cantidad de hilos", argv[0]);
        exit(1);
    }
    N = (long int)pow(2, EXP);

    // Aloca memoria para el vector
    vector = (int *) malloc(sizeof(int) * N);
    if (vector == NULL)
    {
#if DEBUG != 0
        perror("Failed to allocate memory for vector");
#endif
        exit(EXIT_FAILURE);
    }

    sorted_arrs = (int **) malloc(sizeof(int *) * NUM_THREADS);
    if (sorted_arrs == NULL)
    {
#if DEBUG != 0
        perror("Failed to allocate memory for sorted_arrs");
#endif
        exit(EXIT_FAILURE);
    }

    temp_arrs = (int **) malloc(sizeof(int *) * NUM_THREADS);
    if (temp_arrs == NULL)
    {
#if DEBUG != 0
        perror("Failed to allocate memory for temp_arrs");
#endif
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < NUM_THREADS; i++)
    {
        max_size = N / (1 << (int)ceil(log2(i + 1)));
        sorted_arrs[i] = (int *) malloc(sizeof(int) * max_size);
        if (sorted_arrs[i] == NULL)
        {
#if DEBUG != 0
            perror("Failed to allocate memory for sorted_arrs[i]");
#endif
            exit(EXIT_FAILURE);
        }
        temp_arrs[i] = (int *) malloc(sizeof(int) * max_size);
        if (temp_arrs[i] == NULL)
        {
#if DEBUG != 0
            perror("Failed to allocate memory for temp_arrs[i]");
#endif
            exit(EXIT_FAILURE);
        }
    }

    cant_barrs = NUM_THREADS - 1;
    barriers = (pthread_barrier_t *) malloc(sizeof(pthread_barrier_t) * cant_barrs);
    if (barriers == NULL)
    {
#if DEBUG != 0
        perror("Failed to allocate memory for barriers");
#endif
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < cant_barrs; i++)
    {
        pthread_barrier_init(&barriers[i], NULL, 2);
    }

    // Inicializa el vector con valores aleatorios
    srand(time(NULL));
    for (i = 0; i < N; i++)
    {
        vector[i] = rand() % 100;
    }

#if DEBUG != 0
    printf("Given array is \n");
    printArray(vector, N);
#endif

    pthread_t misThreads[NUM_THREADS];
    int threads_ids[NUM_THREADS];

    // Realiza la ordenacion
    timetick = dwalltime();

    for (i = 0; i < NUM_THREADS; i++)
    {
        threads_ids[i] = i;
        pthread_create(&misThreads[i], NULL, &funcion, (void *)&threads_ids[i]);
    }

    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(misThreads[i], NULL);
    }

    printf("\nTime in secs %f\n", dwalltime() - timetick);

    // Verifica el resultado
    for (i = 0; i < N - 1; i++)
    {
        check = check && (vector[i] <= vector[i + 1]);
    }

#if DEBUG != 0
    printf("\nSorted array is \n");
    printArray(vector, N);
#endif

    if (check)
    {
        printf("Success!!\n");
    }
    else
    {
        printf("Sort went wrong:(\n");
    }

    for (i = 0; i < cant_barrs; i++)
    {
        pthread_barrier_destroy(&barriers[i]);
    }
    free(barriers);

    for (i = 0; i < NUM_THREADS; i++)
    {
        free(sorted_arrs[i]);
        free(temp_arrs[i]);
    }
    free(sorted_arrs);
    free(temp_arrs);

    free(vector);
    return (0);
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
    for (long int i = 0; i < size; i++)
        printf("%d ", data[i]);
    printf("\n");
}