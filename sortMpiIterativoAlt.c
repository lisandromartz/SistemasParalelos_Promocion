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

void initAll(long N, int** parteArr, int** aux){
	*parteArr = (int*)malloc(sizeof(int)*N);
    *aux = (int*)malloc(sizeof(int)*N);
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
void mergeSortIterativo(int* parteArr, long N, int *aux) {

    for (long parte = 1; parte <= N-1; parte = 2*parte) {
        for (long inicio = 0; inicio < N-1; inicio += 2*parte) {
            
            long medio = inicio + parte - 1;
            long fin = (inicio + 2*parte - 1 < N-1) ? (inicio + 2*parte - 1) : (N-1);

            // Llamada al merge enviando como parámetro punteros al inicio de las dos mitades
            // Se guarda el arreglo ordenado en temp

            merge(parteArr + inicio, parteArr + medio + 1, parte, aux + inicio);
            
            // Se copia el arreglo temp ordenado en arr
            for (long i = inicio; i <= fin; i++) {
                parteArr[i] = aux[i];
            }
        }
    }
}

// Cuando los procesos terminan de ordenar las partes, los de la mitad
// superior las envían a los de la inferior para que estas las ordenen
void mergeArray(int id, int numProce, int* parteArr, long N, int* aux) {
	int procesosActivos = numProce;
    long parte = N/numProce;
	//int inicio = 0;
	//int medio;
	//int fin;

	while(procesosActivos > 1){
		procesosActivos = procesosActivos/2;

		if(id < procesosActivos) {
            printf("ID: %d\nParte: %ld\nProcesos activos: %d\n", id, parte, procesosActivos);
			// Los procesos que permanecen reciben los arreglos ordenados de los que finalizan
			MPI_Recv(parteArr + parte, parte, MPI_INT,id+procesosActivos,parte, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			/*
            for(int i=0;i<parte;i++) {
				arr[parte+i] = recAux[i];
			}
            */
			// Se duplica el tamaño de las partes y se definen un nuevo fin y medio
			//fin = parte - 1;
			//medio = inicio + (fin - inicio)/2;
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

int main(int argc, char* argv[]){
	int id;
	int numProce;
	
	int *A = NULL, *B = NULL;
	int *parteArr, *aux = NULL;
	long N, parte;
	int check = TRUE;
	double timetick;
	char *ptrFin;
	
	// Se inicia el ambiente MPI, siempre va antes que todo lo demás
	MPI_Init(&argc, &argv);
	// Cada proceso obtiene su ID, se guarda en la variable 'id'
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	// Se obtiene el número de procesos en ejecución, se guarda en 'numProce'
	MPI_Comm_size(MPI_COMM_WORLD, &numProce);

	N = strtol(argv[1], &ptrFin, 10);
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
        #if PRINT == 1
            printf("Arreglos creados\n");
    		printArreglo(A, N);
			printArreglo(B, N);
		#endif
	}
	
	//Alocación de memoria para el arreglo auxiliar
	initAll(N, &aux, &parteArr);

	//int *temp = (int*)malloc(sizeof(int)*N);
	//int *recAux = (int*)malloc(sizeof(int)*N/2);
	
	parte = N/numProce;
	if(aux == NULL) {
        printf("Alocación de parteArr fallada");
        exit(EXIT_FAILURE);
    }

	if(id == 0)
		timetick = dwalltime();

	// Distribución de A entre los procesos
	MPI_Scatter(A, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	mergeSortIterativo(parteArr, parte, aux);
	mergeArray(id, numProce, parteArr, N, aux);

	A = aux;
	/*
	if(id==0) {
		for(int i=0;i<N;i++) {
			A[i] = aux[i];
		}
	}
	*/

    if(id == 0) {
        #if PRINT == 1
			printf("main\n");
            printf("Arreglos ordenados\n");
    		printArreglo(aux, N);
            printArreglo(A, N);
			//printf("Punteros\n");
			//printf("parteArr: %d\n", parteArr);
			//printf("aux: %d\n", aux);
            printf("\n");
		#endif
    }
	
	// Distribución de B entre los procesos
	MPI_Scatter(B, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	mergeSortIterativo(parteArr, parte, aux);
	mergeArray(id, numProce, parteArr, N, aux);
	
	B = aux;

	/*
	if(id==0) {
		for(int i=0;i<N;i++) {
			B[i] = aux[i];
		}
	}
	*/

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

	if(id == 0) {
		free(A);
		//free(B);
	}
	free(parteArr);
	//free(aux);

	MPI_Finalize();
	return(0);
}
