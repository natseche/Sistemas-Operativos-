/**************************************************************
#               Pontificia Universidad Javeriana
#     Autor: Juan Pablo Cañón Contreras
#     Fecha: Mayo 2024
#     Materia: Sistemas Operativos
#     Tema: Taller de Evaluación de Rendimiento
#     Fichero: mm_clasico.c
#	  Objetivo: Documentar el código y modularizar los 3 códigos 
                fuente separándolos de sus funciones y añadiendo
                interfaz
#****************************************************************/

#include "functions.h"  //Incluye el archivo de cabecera con las definiciones de las funciones y variables externas
#include <stdio.h>      //Biblioteca estándar para funciones de entrada/salida
#include <pthread.h>    //Biblioteca para el manejo de hilos POSIX
#include <unistd.h>     //Proporciona acceso a la funcionalidad del sistema operativo POSIX
#include <stdlib.h>     //Contiene funciones para manejo de memoria dinámica, control de procesos y conversiones
#include <time.h>       //Para manejar funciones relacionadas con el tiempo
#include <sys/time.h>   //Proporciona definiciones para estructuras y funciones adicionales de manejo de tiempo

int main(int argc, char *argv[]) {
    if (argc < 3) {
        //Verifica que se hayan proporcionado suficientes argumentos al ejecutar el programa
        printf("Ingreso de argumentos \n $./ejecutable tamMatriz numHilos\n");
        return -1;  //Termina el programa devolviendo un código de error si no se proporcionan los argumentos necesarios
    }
    int SZ = atoi(argv[1]);  //Castea el primer argumento de la línea de comandos a entero para el tamaño de la matriz
    int n_threads = atoi(argv[2]);  //Castea el segundo argumento a entero para el número de hilos

    pthread_t p[n_threads];  //Array para almacenar identificadores de los hilos
    pthread_attr_t atrMM;  //Variable para configurar atributos de los hilos

    //Asignación de las matrices A, B y C a segmentos del bloque de memoria reservado
    mA = MEM_CHUNK;
    mB = mA + SZ*SZ;
    mC = mB + SZ*SZ;

    llenar_matriz(SZ);  //Llena las matrices A y B con valores y establece C en cero
    print_matrix(SZ, mA);  //Imprime la matriz A
    print_matrix(SZ, mB);  //Imprime la matriz B

    inicial_tiempo();  //Captura el tiempo de inicio del procesamiento
    pthread_mutex_init(&MM_mutex, NULL);  //Inicializa el mutex
    pthread_attr_init(&atrMM);  //Inicializa los atributos de los hilos
    pthread_attr_setdetachstate(&atrMM, PTHREAD_CREATE_JOINABLE);  //Configura los hilos como joinables

    //Crea los hilos para realizar la multiplicación de matrices
    for (int j = 0; j < n_threads; j++) {
        struct parametros *datos = (struct parametros *) malloc(sizeof(struct parametros));
        datos->idH = j;  //ID del hilo
        datos->nH = n_threads;  //Número total de hilos
        datos->N = SZ;  //Tamaño de la matriz
        pthread_create(&p[j], &atrMM, mult_thread, (void *)datos);  //Crea el hilo
    }

    //Espera a que todos los hilos terminen su ejecución
    for (int j = 0; j < n_threads; j++)
        pthread_join(p[j], NULL);  //Espera por la finalización del hilo

    final_tiempo();  //Captura el tiempo de finalización y calcula la duración total
    print_matrix(SZ, mC);  //Imprime la matriz resultante C

    //Limpia los recursos utilizados
    pthread_attr_destroy(&atrMM);
    pthread_mutex_destroy(&MM_mutex);
    pthread_exit(NULL);  //Finaliza todos los hilos y limpia los recursos
}
