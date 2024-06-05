#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <math.h>

/*	NOTA: En lugar de recibir por parametros el tamaño N del arreglo cuando se
	lanza el ejecutable, se recibe el parametro EXP que es la potencia de 2 que
	se usara para determinar el tamaño del arreglo */

/*	DEBUG en 1 activa la comprobación de ordenación de ambos arreglos y la
	impresion en pantalla de los arreglos antes y después de ser ordenados */
#define DEBUG 0

/*	SHUFFLE_EQUAL en 1 hace que ambos arreglos tengan los mismos elementos
	pero en distinto orden
	SHUFFLE_EQUAL en 0 hace que ambos arreglos inicializen sus elementos con
	valores aleatorios de manera separada (por lo cual, lo más probable es 
	que terminen con valores distintos)
 */
#define SHUFFLE_EQUAL 1

// Para calcular tiempo
double dwalltime()
{
	double sec;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	sec = tv.tv_sec + tv.tv_usec / 1000000.0;
	return sec;
}

// Funcion para imprimir un arreglo
void printArray(int *data, unsigned long int size)
{
	for (unsigned long int i = 0; i < size; i++)
		printf("%d ", data[i]);
	printf("\n");
}

// Funcion para mezclar los elementos de un arreglo
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

// Funcion para mezclar para mezclar 2 arreglos ordenados del mismo tamaño en un solo arreglo ordenado
void merge(int *L, int *R, unsigned long int size, int *result)
{
	unsigned long int i = 0, j = 0, k = 0;

	// Mezclar los dos arreglos en result
	while (i < size && j < size)
	{
		if (L[i] <= R[j])
		{
			result[k++] = L[i++];
		}
		else
		{
			result[k++] = R[j++];
		}
	}

	// Copiar los elementos remanentes en L, si hay alguno
	while (i < size)
	{
		result[k++] = L[i++];
	}

	// Copiar los elementos remanentes en R, si hay alguno
	while (j < size)
	{
		result[k++] = R[j++];
	}
}

// Funcion de Merge Sort Iterativo
void mergeSort_iterative(int *arr, unsigned long int n, int *temp)
{
	unsigned long int curr_size;
	unsigned long int left_start, mid, right_end;
	unsigned long int i;
	for (curr_size = 1; curr_size <= n - 1; curr_size = 2 * curr_size)
	{
		for (left_start = 0; left_start < n - 1; left_start += 2 * curr_size)
		{
			mid = left_start + curr_size - 1;
			right_end = (left_start + 2 * curr_size - 1 < n - 1) ? (left_start + 2 * curr_size - 1) : (n - 1);

			// Mezcla subarreglos arr[left_start..mid] y arr[mid+1..right_end]
			merge(arr + left_start, arr + mid + 1, curr_size, temp + left_start);

			// Copiar el subarray obtenido en el arreglo original
			for (i = left_start; i <= right_end; i++)
			{
				arr[i] = temp[i];
			}
		}
	}
}

int main(int argc, char *argv[])
{
	unsigned long int N;
	unsigned int EXP = 10;
	int *A, *B;
	int *temp;

	unsigned long int i;
	bool check = true;
	double timetick;

	// Controla los argumentos al programa
	if ((argc != 2) || ((EXP = atoi(argv[1])) <= 0))
	{
		printf("\nUsar: %s x\n    x: Exponente para determinar la cantidad de elementos (2^(x)) de los arreglos", argv[0]);
		exit(1);
	}
	N = (unsigned long int) pow(2, EXP);

	// Se reserva memoria para los arreglos
	A = (int *)malloc(sizeof(int) * N);
	B = (int *)malloc(sizeof(int) * N);

	// Se reserva memoria para los arreglos auxiliares
	temp = (int *)malloc(sizeof(int) * N);

	// Se inicializan los arreglos de manera aleatoria
	srand(time(NULL));
	for (int i = 0; i < N; i++)
	{
		A[i] = rand() % 1000;
#if SHUFFLE_EQUAL != 0
		B[i] = A[i];
#else
		B[i] = rand() % 1000;
#endif
	}

#if SHUFFLE_EQUAL != 0
	shuffle(B, N);
#endif

#if DEBUG != 0
	printf("Array A es \n");
	printArray(A, N);
	printf("\nArray B es \n");
	printArray(B, N);
#endif

	timetick = dwalltime();

	mergeSort_iterative(A, N, temp);
	mergeSort_iterative(B, N, temp);

	for (i = 0; i < N; i++)
	{
		if (A[i] != B[i])
			check = false;
		break;
	}

	printf("\nTiempo en segundos %f\n", dwalltime() - timetick);

#if DEBUG != 0
	printf("\nArray A ordenado es \n");
	printArray(A, N);
	printf("\nArray B ordenado es \n");
	printArray(B, N);

	int check_sortA = true;
	int check_sortB = true;
	for (i = 0; i < N - 1; i++)
	{
		check_sortA = check_sortA && A[i] <= A[i + 1];
		check_sortB = check_sortB && B[i] <= B[i + 1];
	}

	if (check_sortA)
	{
		printf("El arreglo A fue ordenado correctamente\n");
	}
	else
	{
		printf("El arreglo A NO fue ordenado correctamente\n");
	}
	if (check_sortB)
	{
		printf("El arreglo B fue ordenado correctamente\n");
	}
	else
	{
		printf("El arreglo B NO fue ordenado correctamente\n");
	}
#endif

	if (check)
	{
		printf("Los arreglos tienen los mismos elementos\n");
	}
	else
	{
		printf("Los arreglos son diferentes\n");
	}

	free(A);
	free(B);
	free(temp);
	return 0;
}
