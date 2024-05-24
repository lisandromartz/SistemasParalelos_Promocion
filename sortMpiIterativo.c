#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

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

void initRoot(int N, int** A, int** B){
	// Se reserva memoria para los arreglos
	*A = (int*)malloc(sizeof(int)*N);
	*B = (int*)malloc(sizeof(int)*N);
	
    srand (time(NULL));
	for(int i=0;i<N;i++){
		(*A)[i] = rand() % 1000;
		(*B)[i] = rand() % 1000;
	}
}

void initAll(int N, int** parteArr){
	*parteArr = (int*)malloc(sizeof(int)*N);
	//parteA = (int*)malloc(sizeof(int)*N);
	//parteB = (int*)malloc(sizeof(int)*N);
}

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
            // Se guarda el arreglo ordenado en temp

            merge(arr + inicio, arr + medio + 1, parte, temp + inicio);
            
            // Se copia el arreglo temp ordenado en arr
            for (int i = inicio; i <= fin; i++) {
                arr[i] = temp[i];
            }
        }
    }

    free(temp);
}

// Cuando los procesos terminan de ordenar las partes, los de la mitad
// superior las envían a los de la inferior para que estas las ordenen
void mergeArray(int id, int numProce, int* arr, int N) {
	int procesosActivos = numProce;
    int parte = N/numProce;
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
        for (int i = 0; i < N; i++) {
            arr[i] = temp[i];
        }
	}

	free(recAux);
    free(temp);
}

int comparacion(int* A, int* B, int parte) {
	for (int i=0;i<parte; i++) {
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
	int parte;
	int check = TRUE;
	double timetick;
	
	// Se inicia el ambiente MPI, siempre va antes que todo lo demás
	MPI_Init(&argc, &argv);
	// Cada proceso obtiene su ID, se guarda en la variable 'id'
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	// Se obtiene el número de procesos en ejecución, se guarda en 'numProce'
	MPI_Comm_size(MPI_COMM_WORLD, &numProce);
	
	int N = atoi(argv[1]);
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
	initAll(N, &parteArr);
	parte = N/numProce;
	if(parteArr == NULL) {
        printf("Alocación de parteArr fallada");
        exit(EXIT_FAILURE);
    }

	if(id == 0)
		timetick = dwalltime();

	// Distribución de A entre los procesos
	MPI_Scatter(A, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	mergeSortIterativo(parteArr, N);
	mergeArray(id, numProce, parteArr, N);
	if(id==0) {
		for(int i=0;i<N;i++) {
			A[i] = parteArr[i];
		}
	}
	
	// Distribución de B entre los procesos
	MPI_Scatter(B, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	mergeSortIterativo(parteArr, parte);
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
        for (int i = 0; i < N; i++) {
            printf("%d ", A[i]);
        }
        printf("\n");
        for (int i = 0; i < N; i++) {
            printf("%d ", B[i]);
        }
        printf("\n");
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
	
	MPI_Finalize();
	return(0);
}
