/**************************************************************
#               Pontificia Universidad Javeriana
#     Autor: Juan Pablo Cañón Contreras
#     Fecha: Mayo 2024
#     Materia: Sistemas Operativos
#     Tema: Taller de Evaluación de Rendimiento
#     Fichero: mm_transpuesta.c 
#	  Objetivo: Documentar el código y modularizar los 3 códigos 
                fuente separándolos de sus funciones y añadiendo
                interfaz
#****************************************************************/

#include "functions.h"  //Incluir archivo de cabecera con definiciones de funciones y variables
#include <stdio.h>      //Incluir biblioteca estándar de entrada/salida
#include <pthread.h>    //Incluir biblioteca para el manejo de hilos POSIX
#include <unistd.h>     //Incluir biblioteca para interacción con el sistema operativo POSIX
#include <stdlib.h>     //Incluir biblioteca para manejo de memoria y otros utilitarios
#include <time.h>       //Incluir biblioteca para manejo del tiempo
#include <sys/time.h>   //Incluir biblioteca para manejo de tiempo con mayor precisión

int main(int argc, char *argv[]) {
    if (argc < 3) {
        //Verificar el número de argumentos pasados al programa
        printf("Ingreso de argumentos \n $./ejecutable tamMatriz numHilos\n");
        return -1;  //Devolver -1 si no se pasan suficientes argumentos
    }
    int SZ = atoi(argv[1]);  //Castear el primer argumento a entero para el tamaño de la matriz
    int n_threads = atoi(argv[2]);  // Castear el segundo argumento a entero para el número de hilos

    pthread_t p[n_threads];  //Arreglo de identificadores de hilos
    pthread_attr_t atrMM;  //Atributos para los hilos

    //Asignar punteros para matrices A, B y C usando un bloque de memoria grande
    mA = MEM_CHUNK;
    mB = mA + SZ*SZ;
    mC = mB + SZ*SZ;

    llenar_matriz(SZ);  //Llenar las matrices A y B con valores, y C con ceros
    print_matrix(SZ, mA);  //Imprimir la matriz A
    print_matrix(SZ, mB);  //Imprimir la matriz B

    inicial_tiempo();  //Iniciar el temporizador para medir la duración de la operación
    pthread_mutex_init(&MM_mutex, NULL);  //Inicializar el mutex
    pthread_attr_init(&atrMM);  //Inicializar los atributos de los hilos
    pthread_attr_setdetachstate(&atrMM, PTHREAD_CREATE_JOINABLE);  //Establecer los hilos como 'joinable'

    //Crear hilos para realizar la multiplicación de matrices
    for (int j = 0; j < n_threads; j++) {
        struct parametros *datos = (struct parametros *) malloc(sizeof(struct parametros));
        datos->idH = j;  //Identificador del hilo.
        datos->nH = n_threads;  //Total de hilos.
        datos->N = SZ;  //Tamaño de la matriz.
        pthread_create(&p[j], &atrMM, mult_thread, (void *)datos);  //Crear el hilo.
    }

    //Esperar a que todos los hilos completen su trabajo
    for (int j = 0; j < n_threads; j++)
        pthread_join(p[j], NULL);

    final_tiempo();  //Detener el temporizador y mostrar el tiempo transcurrido
    print_matrix(SZ, mC);  //Imprimir la matriz resultante C

    //Limpiar recursos
    pthread_attr_destroy(&atrMM);
    pthread_mutex_destroy(&MM_mutex);
    pthread_exit(NULL);  //Terminar todos los hilos y salir
}
