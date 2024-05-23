/**************************************************************
#               Pontificia Universidad Javeriana
#     Autor: Juan Pablo Cañón Contreras
#     Fecha: Mayo 2024
#     Materia: Sistemas Operativos
#     Tema: Taller de Evaluación de Rendimiento
#     Fichero: functions.c 
#	  Objetivo: Documentar el código y modularizar los 3 códigos 
                fuente separándolos de sus funciones y añadiendo
                interfaz
#****************************************************************/

#include <stdio.h>      //Incluir la biblioteca estándar de entrada y salida
#include <pthread.h>    //Incluir la biblioteca para manejo de hilos POSIX
#include <unistd.h>     //Incluir biblioteca para acceso a funcionalidades del sistema operativo POSIX
#include <stdlib.h>     //Incluir biblioteca estándar para gestión de memoria dinámica y otras funciones
#include <time.h>       //Incluir biblioteca para manejo de tiempo (ej. time())
#include <sys/time.h>   //Incluir biblioteca para funciones y estructuras adicionales de manejo de tiempo
#include "functions.h"  //Incluir archivo de cabecera personalizado para funciones definidas en otro lugar

//Definición global de un mutex para el control de acceso a recursos compartidos entre hilos
pthread_mutex_t MM_mutex;

//Definición de un array estático para almacenar los datos de las matrices
double MEM_CHUNK[DATA_SIZE];

//Punteros para matrices A, B y C
double *mA, *mB, *mC;

//Estructura para medir tiempos de ejecución
struct timeval start, stop;

//Función para llenar las matrices A y B con valores y setear C a 0
void llenar_matriz(int SZ) {
    srand48(time(NULL));  //Inicializa el generador de números aleatorios con la hora actual
    for(int i = 0; i < SZ*SZ; i++) {  //Cicloo para llenar las matrices
        mA[i] = 1.1 * i;  //Asigna valores incrementales multiplicados por 1.1 a A
        mB[i] = 2.2 * i;  //Asigna valores incrementales multiplicados por 2.2 a B
        mC[i] = 0;        //Inicializa todos los elementos de C a 0
    }
}

//Función para imprimir matrices de tamaño pequeño
void print_matrix(int sz, double *matriz) {
    if(sz < 12) {  //Condición para evitar imprimir matrices grandes
        for(int i = 0; i < sz*sz; i++) {
            if(i % sz == 0) printf("\n");  //Imprime un salto de línea al empezar una nueva fila
            printf(" %.3f ", matriz[i]);  //Imprime cada elemento de la matriz con tres decimales
        }
        printf("\n>-------------------->\n");  // Imprime una línea separadora al final
    }
}

//Función para capturar el tiempo de inicio.
void inicial_tiempo() {
    gettimeofday(&start, NULL);  //Obtiene el tiempo actual y lo almacena en 'start'
}

//Función para capturar el tiempo de finalización y calcular la duración
void final_tiempo() {
    gettimeofday(&stop, NULL);  //Obtiene el tiempo de finalización.
    stop.tv_sec -= start.tv_sec;  //Calcula la diferencia en segundos.
    printf("\n%9.0f \n", (double)(stop.tv_sec * 1000000 + stop.tv_usec));  //Imprime la duración en microsegundos
}

//Función ejecutada por cada hilo para realizar la multiplicación de matrices
void *mult_thread(void *variables) {
    struct parametros *data = (struct parametros *)variables;  //Casteo de void* a struct parametros*
    int ini = (data->N / data->nH) * data->idH;  //Calcula el índice inicial para este hilo
    int fin = (data->N / data->nH) * (data->idH + 1);  //Calcula el índice final para este hilo

    for (int i = ini; i < fin; i++) {  //Itera sobre un segmento de filas
        for (int j = 0; j < data->N; j++) {  //Itera sobre columnas
            double *pA = mA + (i * data->N), *pB = mB + j, sumaTemp = 0.0;  //Preparación de punteros y suma
            for (int k = 0; k < data->N; k++, pA++, pB += data->N) {
                sumaTemp += (*pA * *pB);  //Realiza la multiplicación y acumula en sumaTemp
            }
            mC[i * data->N + j] = sumaTemp;  //Asigna el resultado a la matriz C
        }
    }

    pthread_mutex_lock(&MM_mutex);  //Bloquea el mutex para entrar a una sección crítica.
    pthread_mutex_unlock(&MM_mutex);  //Desbloquea el mutex al salir de la sección crítica.
    pthread_exit(NULL);  //Finaliza el hilo.
}
