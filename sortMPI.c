#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include<mpi.h>

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

initAll(int N, int* parteArr){
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
	//int* L = (int*)malloc(sizeof(int)*n1);
	//int* R = (int*)malloc(sizeof(int)*n2);
	
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
	
	//free(L);
	//free(R);
}

void mergeSort(int* array, int inicio, int fin) {
	if(inicio < fin) {
		int medio = inicio + (fin - inicio)/2;

		// Se divide el arreglo en partes recursivamente
		mergeSort(array, inicio, medio);
		mergeSort(array, medio+1, fin);

		merge(array, inicio, medio, fin);
	}
}

fProcesoTipoC(){
	// Función que implementa el comportamiento de los procesos de tipo C
}

int main(int argc, char* argv[]){
	int id;
	int numProce;
	
	int *A, *B;
	int *parteArr;
	int parte;
	bool check = true;
	double timetick;
	
	// Se inicia el ambiente MPI, siempre va antes que todo lo demás
	MPI_Init(&argc, &argv);
	// Cada proceso obtiene su ID, se guarda en la variable 'id'
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	// Se obtiene el número de procesos en ejecución, se guarda en 'numProce'
	MPI_Comm_size(MPI_COMM_WORLD, &numProce);

	int N = atoi(argv[1]);
	if(id == 0) {
		initRoot(N, A, B);
		
		//timetick = dwalltime();
	}
	
	initAll(N, parte, parteArr);
	
	MPI_Scatter(A, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	mergeSort(parteArr, 0, parte-1);
	
	MPI_Scatter(B, parte, MPI_INT, parteArr, parte, MPI_INT, 0, MPI_COMM_WORLD);
	mergeSort(parteArr, 0, parte-1);
	
	//MPI_Gather(parteB, parte, MPI_INT, B, parte, MPI_INT, 0, MPI_COMM_WORLD);
	
	if(id == 0) {
		free(A);
		free(B);
	}
	free(parteArr);
	
	MPI_Finalize();
	return(0);
}
