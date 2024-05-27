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

void initRoot(int N, int* A, int* B){
	// Se reserva memoria para los arreglos
	A = (int*)malloc(sizeof(int)*N);
	B = (int*)malloc(sizeof(int)*N);
	
	srand (time(NULL));
	for(int i=0;i<N;i++){
		A[i] = rand() % 999;
		B[i] = rand() % 999;
	}
}

void initAll(int N, int* parteArr){
	parteArr = (int*)malloc(sizeof(int)*N);
	//parteA = (int*)malloc(sizeof(int)*N);
	//parteB = (int*)malloc(sizeof(int)*N);
}

void merge(int* array, int inicio, int medio, int fin) {
	int n1 = medio - inicio + 1;
	int n2 = fin - medio;
	int i = 0;
	int j = 0;
	int k = inicio;
	
	// Reservar y liberar memoria en cada llamada al merge es super ineficiente
	// Se podría reservar y liberar una sola vez en el main
	int* L = (int*)malloc(sizeof(int)*n1);
	int* R = (int*)malloc(sizeof(int)*n2);
	printf("Llega al merge\n");
	// Se guarda los valores de array en los auxiliares
	for(int i=0;i<n1;i++) {
		L[i] = array[inicio + i];
	}
	for(int j=0;j<n2;j++) {
		R[j] = array[medio + 1 + j];
	}
	
	i = 0;
	j = 0;
	
	// Se ordena el arreglo eligiendo el valor más chico entre L y R, y el elegido avanzaz una posición
	while(i < n1 && j < n2) {
		if(L[i] <= R[j]) {
			array[k] = L[i];
			i++;
		} else {
			array[k] = R[j];
			j++;
		}
		k++;
	}
	
	while(i < n1) {
		array[k] = L[i];
		i++;
		k++;
	}
	while(j < n2) {
		array[k] = R[j];
		j++;
		k++;
	}
	
	free(L);
	free(R);
}
/*
void mergeSortMPI(int id, int numProce, int* array, int parte, int inicio, int fin) {
	int inicio = 0;
	int fin = parte - 1;
	int medio;

	if(inicio < fin) {
		medio = inicio + (fin - inicio)/2;

		// Se divide el arreglo en partes recursivamente
		mergeSortMPI(id, numProce, array, inicio, medio);
		mergeSortMPI(id, numProce, array, medio+1, fin);

		merge(array, inicio, medio, fin);
	}
}
*/

void mergeSort(int* array, int inicio, int fin) {
	if(inicio < fin) {
		int medio = inicio + (fin - inicio)/2;
		printf("MergeSort\n");
		// Se divide el arreglo en partes recursivamente
		mergeSort(array, inicio, medio);
		printf("MergeSort 2\n");
		mergeSort(array, medio+1, fin);
		
		merge(array, inicio, medio, fin);
	}
}

// Cuando los procesos terminan de ordenar las partes, los de la mitad
// superior las envían a los de la inferior para que estas las ordenen
void mergeArray(int id, int numProce, int* array, int parte) {
	int procesosActivos = numProce;
	int* recAux = (int*)malloc(sizeof(int)*(parte*numProce)/2);
	int inicio = 0;
	int medio;
	int fin;

	while(procesosActivos > 1){
		procesosActivos = procesosActivos/2;

		if(id < procesosActivos) {
			// Los procesos que permanecen reciben los arreglos ordenados 
			// de los que finalizan
			MPI_Recv(recAux, parte, MPI_INT,id+procesosActivos,parte, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			for(int i=0;i<parte;i++) {
				array[parte+i] = recAux[i];
			}
			// Se duplica el tamaño de las partes y se definen un nuevo fin y medio
			parte = parte*2;
			fin = parte - 1;
			medio = inicio + (fin - inicio)/2;
			merge(array, inicio, medio, fin);
		} else {
			// Los procesos de la mitad superior de cada ciclo envían su parte a 
			// uno de la mitad inferior
			MPI_Send(array, parte, MPI_INT,id-procesosActivos,parte, MPI_COMM_WORLD);
			break;
		}
	}

	free(recAux);
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
	
	int *A, *B;
	int *parteArr;
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
		initRoot(N, A, B);
		
		timetick = dwalltime();
	}
	
	//Alocación de memoria para el arreglo auxiliar
	initAll(N, parteArr);
	parte = N/numProce;
	
	if(id == 0)
		timetick = dwalltime();

	// Distribución de A entre los procesos
	MPI_Scatter(A, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	printf("Llamada al scatter\n");
	mergeSort(parteArr, 0, parte);
	mergeArray(id, numProce, parteArr, parte);
	if(id==0) {
		for(int i=0;i<N;i++) {
			A[i] = parteArr[i];
		}
	}
	
	// Distribución de B entre los procesos
	MPI_Scatter(B, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	mergeSort(parteArr, 0, parte);
	mergeArray(id, numProce, parteArr, parte);
	if(id==0) {
		for(int i=0;i<N;i++) {
			B[i] = parteArr[i];
		}
	}
	
	int *parteArr2 = (int*)malloc(sizeof(int)*parte);
	MPI_Scatter(A, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(B, parte, MPI_INT, parteArr2, parte, MPI_INT, 0, MPI_COMM_WORLD);

	check = comparacion(parteArr, parteArr2, parte);

	if( check == FALSE) {
		MPI_Scatter(&check, 1, MPI_INT, &check, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}

	if(id == 0) {
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
