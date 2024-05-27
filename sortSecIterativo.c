#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>

#define PRINT 0

long N;
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

// Imprime el arreglo
void printArreglo(int* arr, long N)
{
    for (long i = 0; i < N; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

/*
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
*/
/*
void mergeSort(int* array, int inicio, int fin) {
	if(inicio < fin) {
		int medio = inicio + (fin - inicio)/2;

		// Se divide el arreglo en partes recursivamente
		mergeSort(array, inicio, medio);
		mergeSort(array, medio+1, fin);

		merge(array, inicio, medio, fin);
	}
}
*/
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
void mergeSortIterativo(int* arr, int N, int *temp) {

    for (int parte = 1; parte <= N-1; parte = 2*parte) {
        for (int inicio = 0; inicio < N-1; inicio += 2*parte) {
            
            int medio = inicio + parte - 1;
            int fin = (inicio + 2*parte - 1 < N-1) ? (inicio + 2*parte - 1) : (N-1);

			// Llamada al merge enviando como parámetro punteros al inicio de las dos mitades
            merge(arr + inicio, arr + medio + 1, parte, temp + inicio);
            
            // Se copia el arreglo en temp
            for (int i = inicio; i <= fin; i++) {
                arr[i] = temp[i];
			}
		}
	}
}

// "argc" es la cantidad de argumentos (en este caso 2)
int main(int argc, char* argv[]){
	int i;
	bool check = true;
	double timetick;
	char *ptrFin;
	
	srand (time(NULL));
	// argv[] es un arreglo que contiene los parámetros recibidos en orden
	N = strtol(argv[1], &ptrFin, 10);
	//int potencia = atoi(argv[1]);
	//N = (long int) pow(2, potencia);

	if(*ptrFin != '\0')
		printf("Conversión incorrecta");
	
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
		A[i] = rand() % 1000;
		B[i] = rand() % 1000;
	}
	
	int *temp = (int*)malloc(sizeof(int)*N);

	if (temp == NULL) {
        fprintf(stderr, "Alocación de memoria fallida\n");
        exit(EXIT_FAILURE);
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
	
	mergeSortIterativo(A, N, temp);
	mergeSortIterativo(B, N, temp);
	
	for(i=0;i<N;i++) {
		if(A[i] != B[i])
			check = false;
			break;
	}

	#if PRINT == 1
    		printArreglo(A, N);
			printArreglo(B, N);
	#endif

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
	free(temp);
	return 0;
}
