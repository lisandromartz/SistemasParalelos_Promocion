#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

int N;
int *A, *B;
int *L, *R;

//Para calcular tiempo
double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
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

// "argc" es la cantidad de argumentos (en este caso 2)
int main(int argc, char* argv[]){
	int i;
	bool check = true;
	double timetick;
	
	srand (time(NULL));
	// argv[] es un arreglo que contiene los parámetros recibidos en orden
	N = atoi(argv[1]);
	
	// Se reserva memoria para los arreglos
	A = (int*)malloc(sizeof(int)*N);
	B = (int*)malloc(sizeof(int)*N);
	
	// Se reserva memoria para los arreglos auxiliares
	L = (int*)malloc(sizeof(int)*N);
	R = (int*)malloc(sizeof(int)*N);
	
	for(int i=0;i<N;i++){
		// Si no se reinicia la seed antes de dar valor a B[i], debería dar el mismo que a A
		//srand (time(0));
		//srand (A[i-1]);
		A[i] = rand() % 999;
		B[i] = rand() % 999;
	}
	
	/*
	printf("No ordenado\n");
	printf("%d", A[0]);
	for(i=1;i<N;i++){
		printf(" %d", A[i]);
	}
	printf("\n");
	*/
	
	timetick = dwalltime();
	
	mergeSort(A, 0, N-1);
	mergeSort(B, 0, N-1);
	
	for(i=0;i<N;i++) {
		if(A[i] != B[i])
			check = false;
			break;
	}

	/*
	printf("Ordenado\n");
	printf("%d", A[0]);
	for(i=1;i<N;i++){
		printf(" %d", A[i]);
	}
	printf("\n");

	printf("%d", B[0]);
	for(i=1;i<N;i++){
		printf(" %d", B[i]);
	}
	printf("\n");
	*/
	if(check) {
		printf("Los arreglos tienen los mismos elementos\n");
	} else {
		printf("Los arreglos tienen elementos distintos\n");
	}
	
	printf("Tiempo en segundos %f\n", dwalltime() - timetick);
		
	free(A);
	free(B);
	free(L);
	free(R);
	return 0;
}
