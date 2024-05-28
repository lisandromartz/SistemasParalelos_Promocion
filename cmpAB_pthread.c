#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#define DEBUG 0
#define SHUFFLE_EQUAL 0

// Cantidad de elementos del vector
long int N;

// N = 2^(EXP)
int EXP = 10;

// Cantidad de hilos
int NUM_THREADS;

int *arrA;
int *arrB;

int check = 1;

int **sorted_slicesA;
int **sorted_slicesB;
int **temp_arrs;

// Variables de sincronizacion
pthread_barrier_t *merge_barriers;
pthread_barrier_t cmp_barrier;

void init();
void dispose();
void shuffle(int *, long int);
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
    int barrier_select;
    int check_slice = 1;

    // Copy my respective part of the array
    long int max_size = N / (1 << (int)ceil(log2(id + 1)));
    for (i = 0; i < slice; i++)
    {
        sorted_slicesA[id][i] = arrA[begin + i];
        sorted_slicesB[id][i] = arrB[begin + i];
    }

    // Order my part of the array
    mergeSort_iterative(sorted_slicesA[id], slice, temp_arrs[id]);
    mergeSort_iterative(sorted_slicesB[id], slice, temp_arrs[id]);

    barrier_select = id % (merge_threads / 2);
    pthread_barrier_wait(&merge_barriers[barrier_select]);

    merge_threads /= 2;
    while (id < merge_threads && merge_threads > 1)
    {
        merge(sorted_slicesA[id], sorted_slicesA[id + merge_threads], slice, temp_arrs[id]);
        for (i = 0; i < slice * 2; i++)
        {
            sorted_slicesA[id][i] = temp_arrs[id][i];
        }

        merge(sorted_slicesB[id], sorted_slicesB[id + merge_threads], slice, temp_arrs[id]);
        for (i = 0; i < slice * 2; i++)
        {
            sorted_slicesB[id][i] = temp_arrs[id][i];
        }

        slice *= 2;

        barrier_select = (NUM_THREADS - merge_threads) + id % (merge_threads / 2);
        pthread_barrier_wait(&merge_barriers[barrier_select]);
        merge_threads /= 2;
    }

    if (id == 0)
    {
        merge(sorted_slicesA[id], sorted_slicesA[id + merge_threads], slice, temp_arrs[id]);
        for (i = 0; i < slice * 2; i++)
        {
            arrA[begin + i] = temp_arrs[id][i];
        }

        merge(sorted_slicesB[id], sorted_slicesB[id + merge_threads], slice, temp_arrs[id]);
        for (i = 0; i < slice * 2; i++)
        {
            arrB[begin + i] = temp_arrs[id][i];
        }
    }

    pthread_barrier_wait(&cmp_barrier);

    slice = N / NUM_THREADS;
    i = 0;
    while(check && check_slice && i < slice)
    {
        check_slice = (arrA[begin + i] == arrB[begin + i]);
        i++;
    }
    if(!check_slice){
        check = 0;
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    long int i;
    double timetick;
#if DEBUG != 0
    int check_sortA = 1;
    int check_sortB = 1;
#endif

    // Controla los argumentos al programa
    if ((argc != 3) || ((EXP = atoi(argv[1])) <= 0) || ((NUM_THREADS = atoi(argv[2])) <= 0))
    {
        printf("\nUsar: %s x t\n    x: Exponente para obtener un vector de 2^(x) elementos\n   t: Cantidad de hilos", argv[0]);
        exit(1);
    }

    init();

#if DEBUG != 0
    printf("Given arrA is \n");
    printArray(arrA, N);
    printf("\nGiven arrB is \n");
    printArray(arrB, N);
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
#if DEBUG != 0
    for (i = 0; i < N - 1; i++)
    {
        check_sortA = check_sortA && (arrA[i] <= arrA[i + 1]);
        check_sortB = check_sortB && (arrB[i] <= arrB[i + 1]);
    }

    printf("\nSorted arrA is \n");
    printArray(arrA, N);
    printf("\nSorted arrB is \n");
    printArray(arrB, N);


    if (check_sortA)
    {
        printf("Success in sort A!!\n");
    }
    else
    {
        printf("Sort A went wrong:(\n");
    }

    if (check_sortB)
    {
        printf("Success in sort B!!\n");
    }
    else
    {
        printf("Sort B went wrong:(\n");
    }
#endif
    if(check)
    {
        printf("The arrays are equal");
    }
    else
    {
        printf("The arrays are NOT equal");
    }

    dispose();
    return (0);
}

void init()
{
    N = (long int)pow(2, EXP);

    // Aloca memoria para el vector
    arrA = (int *) malloc(sizeof(int) * N);
    if (arrA == NULL)
    {
        perror("Failed to allocate memory for arrA");
        exit(EXIT_FAILURE);
    }

    arrB = (int *) malloc(sizeof(int) * N);
    if (arrB == NULL)
    {
        perror("Failed to allocate memory for arrB");
        exit(EXIT_FAILURE);
    }

    sorted_slicesA = (int **) malloc(sizeof(int *) * NUM_THREADS);
    if (sorted_slicesA == NULL)
    {
        perror("Failed to allocate memory for sorted_slicesA");
        exit(EXIT_FAILURE);
    }

    sorted_slicesB = (int **) malloc(sizeof(int *) * NUM_THREADS);
    if (sorted_slicesB == NULL)
    {
        perror("Failed to allocate memory for sorted_slicesB");
        exit(EXIT_FAILURE);
    }

    temp_arrs = (int **) malloc(sizeof(int *) * NUM_THREADS);
    if (temp_arrs == NULL)
    {
        perror("Failed to allocate memory for temp_arrs");
        exit(EXIT_FAILURE);
    }

    long int i;
    long int max_size;
    for (i = 0; i < NUM_THREADS; i++)
    {
        max_size = N / (1 << (int)ceil(log2(i + 1)));

        sorted_slicesA[i] = (int *) malloc(sizeof(int) * max_size);
        if (sorted_slicesA[i] == NULL)
        {
            perror("Failed to allocate memory for sorted_slicesA[i]");
            exit(EXIT_FAILURE);
        }

        sorted_slicesB[i] = (int *) malloc(sizeof(int) * max_size);
        if (sorted_slicesB[i] == NULL)
        {
            perror("Failed to allocate memory for sorted_slicesB[i]");
            exit(EXIT_FAILURE);
        }

        temp_arrs[i] = (int *) malloc(sizeof(int) * max_size);
        if (temp_arrs[i] == NULL)
        {
            perror("Failed to allocate memory for temp_arrs[i]");
            exit(EXIT_FAILURE);
        }
    }

    merge_barriers = (pthread_barrier_t *) malloc(sizeof(pthread_barrier_t) * NUM_THREADS - 1);
    if (merge_barriers == NULL)
    {
        perror("Failed to allocate memory for barriers");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < NUM_THREADS - 1; i++)
    {
        pthread_barrier_init(&merge_barriers[i], NULL, 2);
    }
    pthread_barrier_init(&cmp_barrier, NULL, NUM_THREADS);

    // Inicializa el vector con valores aleatorios
    srand(time(NULL));
    for (i = 0; i < N; i++)
    {
        arrA[i] = rand() % 1000;
#if SHUFFLE_EQUAL != 0
        arrB[i] = arrA[i];
#else
        arrB[i] = rand() % 1000;
#endif
    }

#if SHUFFLE_EQUAL != 0
        shuffle(arrA, N);
        shuffle(arrB, N);
#endif    
}

void dispose()
{
    long int i;
    for (i = 0; i < NUM_THREADS - 1; i++)
    {
        pthread_barrier_destroy(&merge_barriers[i]);
    }
    free(merge_barriers);
    pthread_barrier_destroy(&cmp_barrier);

    for (i = 0; i < NUM_THREADS; i++)
    {
        free(sorted_slicesA[i]);
        free(sorted_slicesB[i]);
        free(temp_arrs[i]);
    }
    free(sorted_slicesA);
    free(sorted_slicesB);
    free(temp_arrs);

    free(arrA);
    free(arrB);
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

void shuffle(int *arr, long int size)
{
    int aux;
    srand(time(NULL));
    if(size > 1)
    {
        int rand_i = 0;
        for (long int i = size - 1; i > 1; i--)
        {
            aux = arr[i];
            rand_i = rand() % i;
            arr[i] = arr[rand_i];
            arr[rand_i] = aux;
        }
    }
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