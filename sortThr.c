#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>
#include <pthread.h>

int N, T;
int *A, *B;
int *L[4], *R[4];
volatile bool check = true;
//int cantBar = (int)log2(T);
int cantBar;
pthread_barrier_t* bar;
pthread_barrier_t barFinal;

//Para calcular tiempo
double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}

void merge(int* array, int inicio, int medio, int fin, int id) {
	int n1 = medio - inicio + 1;
	int n2 = fin - medio;
	int i = 0;
	int j = 0;
	int k = inicio;
	
	// Se guarda los valores de array en los auxiliares
	for(int i=0;i<n1;i++) {
		L[id][i] = array[inicio + i];
	}
	for(int j=0;j<n2;j++) {
		R[id][j] = array[medio + 1 + j];
	}
	
	i = 0;
	j = 0;
	
	// Se ordena el arreglo eligiendo el valor más chico entre L y R, y el elegido avanzaz una posición
	while(i < n1 && j < n2) {
		if(L[id][i] <= R[id][j]) {
			array[k] = L[id][i];
			i++;
		} else {
			array[k] = R[id][j];
			j++;
		}
		k++;
	}
	
	while(i < n1) {
		array[k] = L[id][i];
		i++;
		k++;
	}
	while(j < n2) {
		array[k] = R[id][j];
		j++;
		k++;
	}
}

void mergeSort(int* array, int inicio, int fin, int id) {
	if(inicio < fin) {
		int medio = inicio + (fin - inicio)/2;
		
		// Se divide el arreglo en partes recursivamente
		mergeSort(array, inicio, medio, id);
		mergeSort(array, medio+1, fin, id);
		
		merge(array, inicio, medio, fin, id);
	}
}

void* funcion(void *arg){
	int id =* (int*)arg;
	int parte = N/T;
	int hilos = T;
	int b = 0;
	
	int inicio = id*parte;
	int fin = inicio + parte;
	
	while(hilos > 1){
		pthread_barrier_wait(&bar[b]);
		parte = parte*2;
		if(id < hilos/2) {
			mergeSort(&A[id*parte], inicio, fin-1, id);
			mergeSort(&B[id*parte], inicio, fin-1, id);
		} else {
			break;
		}
		hilos = hilos/2;
		b++;
	}
	
	pthread_barrier_wait(&barFinal);
	
	for(int i=inicio;i<fin;i++) {
		// Los otros hilos saldrán del loop cuando check se haga falso
		if(!check)
			break;
		if(A[i] != B[i]) {
			// En teoría no hace falta un mutex, ya que la única modificación posible al check es pasarlo de true a false
			check = false;
			break;
		}
	}
	
	pthread_exit(NULL);
}

// "argc" es la cantidad de argumentos (en este caso 2)
int main(int argc, char* argv[]){
	N = atoi(argv[1]);
	T = atoi(argv[2]);
	cantBar = (int)log2(T);
	int i;
	int div = 1;
	//bool check = true;
	double timetick;

	srand (time(NULL));
	
	bar = malloc(cantBar*sizeof(pthread_barrier_t));
	for(int b=0;b<cantBar;b++){
		pthread_barrier_init(&bar[b], NULL, T/div); // Inicialización de las barreras
		div = div*2;
	}
	pthread_barrier_init(&barFinal, NULL, T);

	// Se reserva memoria para los arreglos
	A = (int*)malloc(sizeof(int)*N);
	B = (int*)malloc(sizeof(int)*N);
	
	// Se reserva memoria para los arreglos auxiliares
	for(int t=0;t<T;t++) {
		L[t] = (int*)malloc(sizeof(int)*N);
		R[t] = (int*)malloc(sizeof(int)*N);
	}
	
	// Inicializa los threads
	pthread_t misThreads[T];
	int threads_ids[T];

	for(int i=0;i<N;i++) {
		A[i] = rand() % 999;
		B[i] = rand() % 999;
	}
	
	/*
	// Arreglo predefinido para prueba
	for(i=0;i<8;i++) {
		A[i] = i;
		B[i] = i;
	}
	*/
	
	timetick = dwalltime();
	
	for(int id=0;id<T;id++){
		threads_ids[id]=id;
		pthread_create(&misThreads[id],NULL,&funcion,(void*)&threads_ids[id]);
	}
	for(int id=0;id<T;id++){
		pthread_join(misThreads[id],NULL);
	}
	
	//mergeSort(A, 0, N-1);
	//mergeSort(B, 0, N-1);
	
	for(i=0;i<N;i++) {
		if(A[i] != B[i]) {
			check = false;
			break;
		}
	}

	printf("Tiempo en segundos %f\n", dwalltime() - timetick);

	if(check) {
		printf("Los arreglos tienen los mismos elementos\n");
	} else {
		printf("Los arreglos tienen elementos distintos\n");
	}
	
	for(int b=0;b<cantBar;b++){
		pthread_barrier_destroy(&bar[b]);
	}
	pthread_barrier_destroy(&barFinal);
	
	free(A);
	free(B);
	for(i=0;i<T;i++) {
		free(L[i]);
		free(R[i]);
	}
	return 0;
}
