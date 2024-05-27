#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>

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
            merge(arr + inicio, arr + medio + 1, parte, temp + inicio);
            
            // Se copia el arreglo en temp
            for (int i = inicio; i <= fin; i++) {
                arr[i] = temp[i];
			}
		}
	}

	free(temp);
}

int main(int argc, char* argv[]){
	int i;
	bool check = true;
	double timetick;
	char *ptrFin;
	
	srand (time(NULL));

	N = strtol(argv[1], &ptrFin, 10);

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
		A[i] = rand() % 1000;
		B[i] = rand() % 1000;
	}

	timetick = dwalltime();
	
	mergeSortIterativo(A, N);
	mergeSortIterativo(B, N);
	
	for(i=0;i<N;i++) {
		if(A[i] != B[i])
			check = false;
			break;
	}

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
