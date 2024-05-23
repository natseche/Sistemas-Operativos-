/**************************************************************
#               Pontificia Universidad Javeriana
#     Autor: Juan Pablo Cañón Contreras
#     Fecha: Mayo 2024
#     Materia: Sistemas Operativos
#     Tema: Taller de Evaluación de Rendimiento
#     Fichero: functions.h 
#	  Objetivo: Documentar el código y modularizar los 3 códigos 
                fuente separándolos de sus funciones y añadiendo
                interfaz
#****************************************************************/

//Si no está definido, definir la interfaz functions.h
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>      //Incluye la biblioteca estándar de entrada/salida para funciones como printf
#include <pthread.h>    //Incluye las funciones y tipos para el manejo de hilos POSIX
#include <unistd.h>     //Proporciona acceso a la API de POSIX para operaciones del sistema operativo
#include <stdlib.h>     //Incluye funciones de gestión de memoria dinámica y otras utilidades
#include <time.h>       //Permite el uso de funciones relacionadas con el manejo del tiempo
#include <sys/time.h>   //Incluye definiciones para estructuras y funciones adicionales de manejo de tiempo

#define DATA_SIZE (1024*1024*64*3)  //Define una constante para el tamaño total de los datos de las matrices

//Declara extern para indicar que estas variables están definidas en otro lugar (por ejemplo, en un archivo .c)
extern pthread_mutex_t MM_mutex;   //Mutex para control de acceso concurrente
extern double MEM_CHUNK[];         //Array que almacenará las matrices A, B y C como bloques de memoria continuos
extern double *mA, *mB, *mC;       //Punteros para las matrices A, B y C

//Estructura para pasar parámetros a las funciones de los hilos
struct parametros {
    int nH;    //Número total de hilos
    int idH;   //ID específico de este hilo
    int N;     //Dimensión de la matriz (NxN)
};

//Variables extern para medir tiempos de ejecución, definidas también en otro archivo
extern struct timeval start, stop;

//Declaraciones de funciones que se definen en otro archivo .c
//Esto le dice al compilador qué funciones esperar, pero no sus implementaciones específicas
void llenar_matriz(int SZ);                  //Función para inicializar las matrices A y B, y setear C a 0
void print_matrix(int sz, double *matriz);   //Función para imprimir matrices de un tamaño específico
void inicial_tiempo();                       //Función para marcar el inicio de un período de medición de tiempo
void final_tiempo();                         //Función para marcar el fin del período de medición de tiempo y calcular la duración
void *mult_thread(void *variables);          //Función que los hilos ejecutarán para multiplicar matrices

#endif  //Cierra la condición de preprocesador iniciada con #ifndef FUNCTIONS_H.
