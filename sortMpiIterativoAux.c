#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <mpi.h>

#define PRINT 0

#define TRUE 1
#define FALSE 0

//Para calcular tiempo
double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}

// Imprime el arreglo
void printArreglo(int* arr, long N)
{
    for (long i = 0; i < N; i++)
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
		//(*B)[i] = rand() % 1000;
		(*B)[N-i-1] = (*A)[i];
	}
}

void initAll(long N, int id, int** parteArr) {
	long max_size = N / (1 << (int)ceil(log2(id + 1)));
	*parteArr = (int*)malloc(sizeof(int)*max_size);
	printf("ID: %d, Tamaño del arreglo: %ld\n", id, max_size);
	//parteA = (int*)malloc(sizeof(int)*N);
	//parteB = (int*)malloc(sizeof(int)*N);
}

// Se unen los arreglos en uno solo ordenado
void merge(int *L, int *R, long parte, int *result) {
    long i = 0, j = 0, k = 0;

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
void mergeSortIterativo(int* arr, long N, int *temp) {

    for (long parte = 1; parte <= N-1; parte = 2*parte) {
        for (long inicio = 0; inicio < N-1; inicio += 2*parte) {
            
            long medio = inicio + parte - 1;
            long fin = (inicio + 2*parte - 1 < N-1) ? (inicio + 2*parte - 1) : (N-1);

            // Llamada al merge enviando como parámetro punteros al inicio de las dos mitades
            // Se guarda el arreglo ordenado en temp

            merge(arr + inicio, arr + medio + 1, parte, temp + inicio);
            
            // Se copia el arreglo temp ordenado en arr
            for (long i = inicio; i <= fin; i++) {
                arr[i] = temp[i];
            }
        }
    }
}

// Cuando los procesos terminan de ordenar las partes, los de la mitad
// superior las envían a los de la inferior para que estas las ordenen
void mergeArray(int id, int numProce, int* arr, long N) {
	int procesosActivos = numProce;
    long parte = N/numProce;
	int* recAux = (int*)malloc(sizeof(int)*N/2);
    int* temp = (int*)malloc(sizeof(int)*N);
	//int inicio = 0;
	//int medio;
	//int fin;

	while(procesosActivos > 1){
		procesosActivos = procesosActivos/2;

		if(id < procesosActivos) {
			// Los procesos que permanecen reciben los arreglos ordenados de los que finalizan
			MPI_Recv(recAux, parte, MPI_INT,id+procesosActivos,parte, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			/*
            for(int i=0;i<parte;i++) {
				arr[parte+i] = recAux[i];
			}
            */
			// Se duplica el tamaño de las partes y se definen un nuevo fin y medio
			//fin = parte - 1;
			//medio = inicio + (fin - inicio)/2;
			merge(arr, recAux, parte, temp);
            parte = parte*2;
		} else {
			// Los procesos de la mitad superior de cada ciclo envían su parte a uno de la mitad inferior
			MPI_Send(arr, parte, MPI_INT,id-procesosActivos,parte, MPI_COMM_WORLD);
			break;
		}
        
        // Se copia el arreglo temp ordenado en arr
        for (long i = 0; i < N; i++) {
            arr[i] = temp[i];
        }
	}

	free(recAux);
    free(temp);
}

int comparacion(int* A, int* B, long parte) {
	for (long i=0;i<parte; i++) {
		if(A[i] != B[i]) {
			return FALSE;
		}
	}
	return TRUE;
}

int main(int argc, char* argv[]){
	int id;
	int numProce;
	
	int *A = NULL, *B = NULL;
	int *parteArr = NULL;
	long parte;
	int check = TRUE;
	double timetick;
	char *ptrFin;
	
	// Se inicia el ambiente MPI, siempre va antes que todo lo demás
	MPI_Init(&argc, &argv);
	// Cada proceso obtiene su ID, se guarda en la variable 'id'
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	// Se obtiene el número de procesos en ejecución, se guarda en 'numProce'
	MPI_Comm_size(MPI_COMM_WORLD, &numProce);

	long N = strtol(argv[1], &ptrFin, 10);
	//long N = (long) pow(2, atoi(argv[1]));
	
	if(*ptrFin != '\0')
		printf("Conversión incorrecta");

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
		timetick = dwalltime();
	}
	
	//Alocación de memoria para el arreglo auxiliar
	initAll(N, id, &parteArr);

	int *temp = (int*)malloc(sizeof(int)*N);

	parte = N/numProce;
	if(parteArr == NULL) {
        printf("Alocación de parteArr fallada");
        exit(EXIT_FAILURE);
    }

	if(id == 0)
		timetick = dwalltime();

	// Distribución de A entre los procesos
	MPI_Scatter(A, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	mergeSortIterativo(parteArr, parte, temp);
	mergeArray(id, numProce, parteArr, N);
	if(id==0) {
		for(int i=0;i<N;i++) {
			A[i] = parteArr[i];
		}
	}
	
	// Distribución de B entre los procesos
	MPI_Scatter(B, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	mergeSortIterativo(parteArr, parte, temp);
	mergeArray(id, numProce, parteArr, N);
	if(id==0) {
		for(int i=0;i<N;i++) {
			B[i] = parteArr[i];
		}
	}

	int *parteArr2 = (int*)malloc(sizeof(int)*parte);
	MPI_Scatter(A, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(B, parte, MPI_INT, parteArr2, parte, MPI_INT, 0, MPI_COMM_WORLD);

	check = comparacion(parteArr, parteArr2, parte);
    
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

	if(id == 0) {
		free(A);
		free(B);
	}
	free(parteArr);
	free(parteArr2);
	free(temp);

	MPI_Finalize();
	return(0);
}
