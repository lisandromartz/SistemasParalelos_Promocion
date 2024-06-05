#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

/*	NOTA: En lugar de recibir por parametros el tamaño N del arreglo cuando se
	lanza el ejecutable, se recibe el parametro EXP que es la potencia de 2 que
	se usara para determinar el tamaño del arreglo */

/*	DEBUG en 1 activa la comprobación de ordenación de ambos arreglos y la
	impresion en pantalla de los arreglos antes y después de ser ordenados */
#define DEBUG 0

/*	SHUFFLE_EQUAL en 1 hace que ambos arreglos tengan los mismos elementos
	pero en distinto orden
	SHUFFLE_EQUAL en 0 hace que ambos arreglos inicializen sus elementos con
	valores aleatorios de manera separada (por lo cual, lo más probable es 
	que terminen con valores distintos)
 */
#define SHUFFLE_EQUAL 1

// Cantidad de elementos del vector
unsigned long int N;

// N = 2^(EXP)
unsigned int EXP = 10;

// Cantidad de hilos
unsigned int NUM_THREADS;

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
void shuffle(int *, unsigned long int);
double dwalltime();
void merge(int *, int *, unsigned long int, int *);
void mergeSort_iterative(int *, unsigned long int, int *);
void printArray(int *, unsigned long int);

void *funcion(void *arg)
{
    int id = *(int *)arg;
#if DEBUG != 0
    printf("Thread id: %d\n", id);
#endif

    unsigned long int i;
    unsigned int merge_threads = NUM_THREADS;
    unsigned long int slice = N / NUM_THREADS;
    unsigned long int begin = id * slice;
    unsigned int barrier_select;
    int check_slice = 1;

    // Copiar las partes de cada arreglo asignadas a este hilo
    unsigned long int max_size = N / (1 << (int)ceil(log2(id + 1)));
    for (i = 0; i < slice; i++)
    {
        sorted_slicesA[id][i] = arrA[begin + i];
        sorted_slicesB[id][i] = arrB[begin + i];
    }

    // Ordenar las partes de este hilo
    mergeSort_iterative(sorted_slicesA[id], slice, temp_arrs[id]);
    mergeSort_iterative(sorted_slicesB[id], slice, temp_arrs[id]);

    // Sincronizar con hilo de la mitad inferior/superior
    barrier_select = id % (merge_threads / 2);
    pthread_barrier_wait(&merge_barriers[barrier_select]);

    int *aux;
    merge_threads /= 2;
    // Continuar si soy un hilo de la mitad inferior
    while (id < merge_threads && merge_threads > 1)
    {
        // Mezclar las partes de este hilo con las del hilo superior
        merge(sorted_slicesA[id], sorted_slicesA[id + merge_threads], slice, temp_arrs[id]);
        aux = sorted_slicesA[id];
        sorted_slicesA[id] = temp_arrs[id];
        temp_arrs[id] = aux;

        merge(sorted_slicesB[id], sorted_slicesB[id + merge_threads], slice, temp_arrs[id]);
        aux = sorted_slicesB[id];
        sorted_slicesB[id] = temp_arrs[id];
        temp_arrs[id] = aux;

        slice *= 2;

        // Sincronizar con hilo de la mitad inferior/superior
        barrier_select = (NUM_THREADS - merge_threads) + id % (merge_threads / 2);
        pthread_barrier_wait(&merge_barriers[barrier_select]);
        merge_threads /= 2;
    }

    if (id == 0)
    {
        merge(sorted_slicesA[id], sorted_slicesA[id + merge_threads], slice, temp_arrs[id]);
        aux = arrA;
        arrA = temp_arrs[id];
        temp_arrs[id] = aux;

        merge(sorted_slicesB[id], sorted_slicesB[id + merge_threads], slice, temp_arrs[id]);
        aux = arrB;
        arrB = temp_arrs[id];
        temp_arrs[id] = aux;
    }

    pthread_barrier_wait(&cmp_barrier);

    // Realizar igualdad entre secciones
    slice = N / NUM_THREADS;
    i = 0;
    while(check_slice && i < slice)
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
    unsigned long int i;
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
    printf("Array arrA es \n");
    printArray(arrA, N);
    printf("\nArray arrB es \n");
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

    printf("\nTiempo en segundos %f\n", dwalltime() - timetick);

    // Verifica el resultado
#if DEBUG != 0
    for (i = 0; i < N - 1; i++)
    {
        check_sortA = check_sortA && (arrA[i] <= arrA[i + 1]);
        check_sortB = check_sortB && (arrB[i] <= arrB[i + 1]);
    }

    printf("\nArray arrA ordenado es \n");
    printArray(arrA, N);
    printf("\nArray arrB ordenado es \n");
    printArray(arrB, N);


    if (check_sortA)
    {
        printf("El arreglo arrA fue ordenado correctamente\n");
    }
    else
    {
        printf("El arreglo arrA NO fue ordenado correctamente\n");
    }

    if (check_sortB)
    {
        printf("El arreglo arrB fue ordenado correctamente\n");
    }
    else
    {
        printf("El arreglo arrB NO fue ordenado correctamente\n");
    }
#endif
    if(check)
    {
        printf("Los arreglos tienen los mismos elementos\n");
    }
    else
    {
        printf("Los arreglos son diferentes\n");
    }

    dispose();
    return (0);
}

void init()
{
    N = (unsigned long int)pow(2, EXP);

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

    unsigned long int i;
    unsigned long int max_size;
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

    // Inicializa los arreglos con valores aleatorios
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
        shuffle(arrB, N);
#endif    
}

void dispose()
{
    unsigned int i;
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

// Funcion para mezclar los elementos de un arreglo
void shuffle(int *arr, unsigned long int size)
{
    int aux;
    srand(time(NULL));
    if(size > 1)
    {
        unsigned long int rand_i = 0;
        for (unsigned long int i = size - 1; i > 0; i--)
        {
            aux = arr[i];
            rand_i = rand() % i;
            arr[i] = arr[rand_i];
            arr[rand_i] = aux;
        }
    }
}

// Funcion para mezclar para mezclar 2 arreglos ordenados del mismo tamaño en un solo arreglo ordenado
void merge(int *L, int *R, unsigned long int size, int *result)
{
    unsigned long int i = 0, j = 0, k = 0;

    // Mezclar los dos arreglos en result
    while (i < size && j < size)
    {
        if (L[i] <= R[j])
        {
            result[k++] = L[i++];
        }
        else
        {
            result[k++] = R[j++];
        }
    }

    // Copiar los elementos remanentes en L, si hay alguno
    while (i < size)
    {
        result[k++] = L[i++];
    }

    // Copiar los elementos remanentes en R, si hay alguno
    while (j < size)
    {
        result[k++] = R[j++];
    }
}

// Iterative merge sort function
void mergeSort_iterative(int *arr, unsigned long int n, int *temp)
{
    unsigned long int curr_size;
    unsigned long int left_start, mid, right_end;
    unsigned long int i;
    for (curr_size = 1; curr_size <= n - 1; curr_size = 2 * curr_size)
    {
        for (left_start = 0; left_start < n - 1; left_start += 2 * curr_size)
        {
            mid = left_start + curr_size - 1;
            right_end = (left_start + 2 * curr_size - 1 < n - 1) ? (left_start + 2 * curr_size - 1) : (n - 1);

            // Mezcla subarreglos arr[left_start..mid] y arr[mid+1..right_end]
            merge(arr + left_start, arr + mid + 1, curr_size, temp + left_start);

            // Copiar el subarray obtenido en el arreglo original
            for (i = left_start; i <= right_end; i++)
            {
                arr[i] = temp[i];
            }
        }
    }
}

// Funcion para imprimir un arreglo
void printArray(int *data, unsigned long int size)
{
    for (unsigned long int i = 0; i < size; i++)
        printf("%d ", data[i]);
    printf("\n");
}