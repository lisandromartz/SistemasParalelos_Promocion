#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

// Cantidad de elementos del vector
int N = 100;

// Cantidad de hilos
int T;

// Vector a ordenar
int *vector;

int** sorted_arrs;

// Variables de sincronizacion
pthread_barrier_t *barriers;

double dwalltime();

void mergeSort_iterative(int *, int);
void merge(int *, int *, int, int *);
void printArray(int *, int);

void *funcion(void *arg)
{
    int id = *(int *)arg;
    printf("Hilo id: %d\n", id);

    int hilos = T;
    int parte = N / T;
    int inicio = id * parte;
    //Each process allocates only the memory it'll require
    int max_size = N / (1 << (int) ceil(log2(id + 1))); 

    sorted_arrs[id] = (int*) malloc(sizeof(int) * max_size);
    if (sorted_arrs[id] == NULL) {
        perror("Failed to allocate memory for sorted_arrs[id]");
        pthread_exit(NULL);
    }

	int* temp = (int*) malloc(max_size * sizeof(int));
    if (temp == NULL) {
        perror("Failed to allocate memory for temp");
        exit(EXIT_FAILURE);
    }

    int i;
    for (i = 0; i < max_size; i++)
    {
        if(i >= parte)
        {
            sorted_arrs[id][i] = 0;
        }
        else
        {
            sorted_arrs[id][i] = vector[inicio + i];
        }
    }
    
    // Each process orders its part of the array
    // mergeSort_iterative(vector + inicio, parte);
    mergeSort_iterative(sorted_arrs[id], parte);

	int barrier_select = 0;
	pthread_barrier_wait(&barriers[barrier_select]);

    hilos /= 2;
    while (id < hilos && hilos > 1)
    {
		// Merge the subarray ordered by one 'left' process with 
        // the one ordered by its 'right' process counterpart
        merge(sorted_arrs[id], sorted_arrs[id + hilos], parte, temp);

        parte *= 2;
        for (i = 0; i < parte; i++)
		{
			sorted_arrs[id][i] = temp[i];
		}

		barrier_select++;
		pthread_barrier_wait(&barriers[barrier_select]);
		hilos /= 2;
    }

	if(id == 0)
	{
        merge(sorted_arrs[id], sorted_arrs[id + hilos], parte, temp);
        parte *= 2;
		for (int i = 0; i < parte; i++)
		{
			vector[inicio + i] = temp[i];
        }
	}

    free(temp);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int i, j;
    double promedio;
    double timetick;
    int check = 1;

    // Controla los argumentos al programa
    if ((argc != 3) || ((N = atoi(argv[1])) <= 0) || ((T = atoi(argv[2])) <= 0))
    {
        printf("\nUsar: %s x t\n  n: Dimension del vector\n t: Cantidad de hilos", argv[0]);
        exit(1);
    }

    // Aloca memoria para el vector
    vector = (int *)malloc(sizeof(int) * N);
    if (vector == NULL) {
        perror("Failed to allocate memory for vector");
        exit(EXIT_FAILURE);
    }

    sorted_arrs = (int**) malloc(sizeof(int *) * T);
    if (sorted_arrs == NULL) {
        perror("Failed to allocate memory for sorted_arrs");
        exit(EXIT_FAILURE);
    }

    int cant_barrs = (int) log2(T);
    barriers = (pthread_barrier_t *) malloc(sizeof(pthread_barrier_t) * cant_barrs);

    // Inicializa el vector con valores aleatorios
    srand(time(NULL));
    for (i = 0; i < N; i++)
    {
        vector[i] = rand() % 100;
    }

    printf("Arreglo desordenado es \n");
    printArray(vector, N);

    pthread_t misThreads[T];
    int threads_ids[T];

    int div = 1;
    for (i = 0; i < cant_barrs; i++)
    {
        pthread_barrier_init(&barriers[i], NULL, T / div);
        div *= 2;
    }

    // Realiza la ordenacion
    timetick = dwalltime();

    for (int id = 0; id < T; id++)
    {
        threads_ids[id] = id;
        pthread_create(&misThreads[id], NULL, &funcion, (void *)&threads_ids[id]);
    }

    for (int id = 0; id < T; id++)
    {
        pthread_join(misThreads[id], NULL);
    }

    printf("\nTiempo en segundos %f\n", dwalltime() - timetick);

    // Verifica el resultado
    for (i = 0; i < N - 1; i++)
    {
        check = check && (vector[i] <= vector[i + 1]);
    }

    printf("Arreglo ordenado es \n");
    printArray(vector, N);

    if (check)
    {
        printf("Ordenacion del vector resultado correcto\n");
    }
    else
    {
        printf("Ordenacion del vector resultado erroneo\n");
    }

    for (i = 0; i < cant_barrs; i++)
    {
        pthread_barrier_destroy(&barriers[i]);
    }

    free(barriers);

    for (i = 0; i < T; i++)
    {
        free(sorted_arrs[i]);
    }
    free(sorted_arrs);

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

#include <stdio.h>
#include <stdlib.h>

// Function to merge two sorted arrays of the same size into a single sorted array
void merge(int *arr1, int *arr2, int size, int *result)
{
    int i = 0, j = 0, k = 0;

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
void mergeSort_iterative(int *arr, int n)
{
    int *temp = (int *)malloc(n * sizeof(int));
    if (temp == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (int curr_size = 1; curr_size <= n - 1; curr_size = 2 * curr_size)
    {
        for (int left_start = 0; left_start < n - 1; left_start += 2 * curr_size)
        {
            int mid = left_start + curr_size - 1;
            int right_end = (left_start + 2 * curr_size - 1 < n - 1) ? (left_start + 2 * curr_size - 1) : (n - 1);

            // Merge subarrays arr[left_start..mid] and arr[mid+1..right_end]
            merge(arr + left_start, arr + mid + 1, curr_size, temp + left_start);

            // Copy the merged subarray back to the original array
            for (int i = left_start; i <= right_end; i++)
            {
                arr[i] = temp[i];
            }
        }
    }

    free(temp);
}

// Function to print an array
void printArray(int *data, int size)
{
    for (int i = 0; i < size; i++)
        printf("%d ", data[i]);
    printf("\n");
}