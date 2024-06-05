#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>

// NOTA: En lugar de recibir por parametros el tamaño N del arreglo cuando se lanza el ejecutable, se recibe el parametro EXP que es la potencia de 2 que se usara para determinar el tamaño del arreglo

#define DEBUG 0
#define SHUFFLE_EQUAL 1
// PRINT incluye la impresión de los arreglos completos, no incluir para N grandes
#define PRINT 0

#define TRUE 1
#define FALSE 0

// Para calcular tiempo
double dwalltime()
{
	double sec;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	sec = tv.tv_sec + tv.tv_usec / 1000000.0;
	return sec;
}

// Imprime el arreglo
void printArreglo(int* arr, unsigned long int N) {
    for (unsigned long int i = 0; i < N; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

void initRoot(long N, int** A, int** B){
	// Se reserva memoria para los arreglos
	*A = (int*)malloc(sizeof(int)*N);
	*B = (int*)malloc(sizeof(int)*N);
	
    srand (time(NULL));
	for(int i=0;i<N;i++){
		(*A)[i] = rand() % 1000;
		(*B)[N-i-1] = (*A)[i];
	}
}

void initAll(long N, int** parteArr, int** aux){
	*parteArr = (int*)malloc(sizeof(int)*N);
    *aux = (int*)malloc(sizeof(int)*N);
}

// Función para mezclar los elementos de los arreglos
void shuffle(int *arr, unsigned long int size)
{
	int aux;
	srand(time(NULL));
	if (size > 1)
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

// Se combinan dos arreglos en uno solo ordenado
void merge(int *L, int *R, unsigned long int size, int *result) {
	unsigned long int i = 0, j = 0, k = 0;

	while (i < size && j < size) {
		if (L[i] <= R[j]) {
			result[k++] = L[i++];
		} else {
			result[k++] = R[j++];
		}
	}

	// Copia los elementos restantes de L
	while (i < size) {
		result[k++] = L[i++];
	}

	// Copia los elementos restantes de R
	while (j < size) {
		result[k++] = R[j++];
	}
}

void mergeSortIterativo(int *arr, unsigned long int N, int *temp) {
	unsigned long int curr_size;
	unsigned long int left_start, mid, right_end;
	unsigned long int i;
	for (curr_size = 1; curr_size <= N - 1; curr_size = 2 * curr_size) {
		for (left_start = 0; left_start < N - 1; left_start += 2 * curr_size) {
			mid = left_start + curr_size - 1;
			right_end = (left_start + 2 * curr_size - 1 < N - 1) ? (left_start + 2 * curr_size - 1) : (N - 1);

			merge(arr + left_start, arr + mid + 1, curr_size, temp + left_start);

			for (i = left_start; i <= right_end; i++) {
				arr[i] = temp[i];
			}
		}
	}
}

void mergeArray(int id, int numProce, int* parteArr, long N, int* aux) {
	int procesosActivos = numProce;
    long parte = N/numProce;

	while(procesosActivos > 1){
		procesosActivos = procesosActivos/2;

		if(id < procesosActivos) {
			#if PRINT == 1
            	printf("ID: %d\nParte: %ld\nProcesos activos: %d\n", id, parte, procesosActivos);
			#endif
			// Los procesos que permanecen reciben los arreglos ordenados de los que finalizan
			MPI_Recv(parteArr + parte, parte, MPI_INT,id+procesosActivos,parte, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			
			#if PRINT == 1
                printf("parteArr NO ordenada\n");
    		    printArreglo(parteArr, parte);
    	    #endif
			
			merge(parteArr, parteArr + parte, parte, aux);
            int* aux2 = parteArr;
            parteArr = aux;
            aux = aux2;
            parte = parte*2;

            #if PRINT == 1
		printf("mergeArray\n");
                printf("parteArr ordenada\n");
		printArreglo(parteArr, parte);
		//printf("Punteros\n");
		//printf("parteArr: %d\n", parteArr);
		//printf("aux: %d\n", aux);
                printf("\n");
		#endif

		} else {
			// Los procesos de la mitad superior de cada ciclo envían su parte a uno de la mitad inferior
			MPI_Send(parteArr, parte, MPI_INT,id-procesosActivos,parte, MPI_COMM_WORLD);
			break;
		}

        aux = parteArr;
	}
}

int comparacion(int* A, int* B, long parte) {
	for (long i=0;i<parte; i++) {
		if(A[i] != B[i]) {
			return FALSE;
		}
	}
	return TRUE;
}

int main(int argc, char *argv[]) {
	int id;
	int numProce;

	unsigned long int N, parte;
	unsigned int EXP = 10;
	int *A = NULL, *B = NULL;
	int *parteArr = NULL, *aux=NULL;
	int check = TRUE;
	unsigned long int i;
	double timetick;

	// Se inicia el ambiente MPI, siempre va antes que todo lo demás
	MPI_Init(&argc, &argv);
	// Cada proceso obtiene su ID, se guarda en la variable 'id'
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	// Se obtiene el número de procesos en ejecución, se guarda en 'numProce'
	MPI_Comm_size(MPI_COMM_WORLD, &numProce);

	// Controla los argumentos al programa
	if ((argc != 2) || ((EXP = atoi(argv[1])) <= 0))
	{
		printf("\nUsar: %s x\n    x: Exponente para determinar la cantidad de elementos (2^(x)) de los arreglos", argv[0]);
		exit(1);
	}
	N = (unsigned long int) pow(2, EXP);

	#if PRINT == 1
            printf("Tamaño: %ld\n\n", N);
    #endif

	// Alocación de memoria para A y B
	if(id == 0) {
		initRoot(N, &A, &B);
		if(A == NULL) {
            printf("Alocación de A fallada");
            exit(EXIT_FAILURE);
        }
        if(B == NULL) {
            printf("Alocación de B fallada");
            exit(EXIT_FAILURE);
        }
        #if PRINT == 1
            printf("Arreglos creados\n");
    		printArreglo(A, N);
			printArreglo(B, N);
		#endif
	}
	
	//Alocación de memoria para el arreglo auxiliar
	initAll(N, &aux, &parteArr);
	parte = N/numProce;
	
	if(id == 0)
		timetick = dwalltime();

	// Distribución de A entre los procesos
	MPI_Scatter(A, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	mergeSortIterativo(parteArr, parte, aux);
	mergeArray(id, numProce, parteArr, N, aux);

	A = aux;

    if(id == 0) {
        #if PRINT == 1
		printf("main\n");
		printf("Arreglos ordenados\n");
		printArreglo(aux, N);
		printArreglo(A, N);
		printf("\n");
	#endif
    }
	
	// Distribución de B entre los procesos
	MPI_Scatter(B, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	mergeSortIterativo(parteArr, parte, aux);
	mergeArray(id, numProce, parteArr, N, aux);
	
	B = aux;

	MPI_Scatter(A, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(B, parte, MPI_INT, aux, parte, MPI_INT, 0, MPI_COMM_WORLD);

	check = comparacion(parteArr, aux, parte);
    
	if(check == FALSE) {
		MPI_Allreduce(MPI_IN_PLACE, &check, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
	}

	if(id == 0) {
		#if PRINT == 1
    		printArreglo(A, N);
			printArreglo(B, N);
		#endif
		printf("Tiempo en segundos %f\n", dwalltime() - timetick);

		if(check == TRUE) {
			printf("Los arreglos tienen los mismos elementos\n");
		} else {
			printf("Los arreglos tienen elementos distintos\n");
		}
	}

	#if DEBUG != 0
		printArray(A, N);
		printArray(B, N);

		int check_sortA = true;
		int check_sortB = true;
		for (i = 0; i < N - 1; i++) {
			check_sortA = check_sortA && A[i] <= A[i + 1];
			check_sortB = check_sortB && B[i] <= B[i + 1];
		}

		if (check_sortA) {
			printf("El arreglo A fue ordenado correctamente\n");
		} else {
			printf("El arreglo A NO fue ordenado correctamente\n");
		}
		if (check_sortB) {
			printf("El arreglo B fue ordenado correctamente\n");
		} else {
			printf("El arreglo B NO fue ordenado correctamente\n");
		}
	#endif

	if(id == 0) {
		free(A);
	}
	free(parteArr);

	MPI_Finalize();
	return 0;
}
