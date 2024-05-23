/**************************************************************
#               Pontificia Universidad Javeriana
#     Autor: Natalia Echeverry Salcedo, Maria Paula Rodriguez, Daniel Carvajal y Juan Pablo Cañón Contreras
#     Fecha: Mayo 2024
#     Materia: Sistemas Operativos
#     Tema: Proyecto Sistemas Operativos 
#     Fichero: monitor.c
#****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/stat.h> //Para la función mkfifo

//Definimos las banderas para temperatura y pH
#define TEMPERATURA_FLAG "TEMP"
#define PH_FLAG "PH"

//Colores para la consola
const char *RED = "\033[31m";
const char *GREEN = "\033[32m";
const char *YELLOW = "\033[33m";
const char *BLUE = "\033[34m";
const char *RESET = "\033[0m"; 

//Tamaño del buffer para almacenar datos temporales
int BUFFER_SIZE;  

//Semáforos para controlar la disponibilidad de espacio y datos en los buffers
sem_t empty_temp, full_temp, empty_ph, full_ph; 

//Mutexes para proteger el acceso a los buffers
pthread_mutex_t mutex_temp, mutex_ph;  

//Buffer para almacenar datos de temperatura
char **buffer_temp; 

//Buffer para almacenar datos de pH
char **buffer_ph;  

//Índices para los buffers de entrada y salida
int in_temp = 0, out_temp = 0, in_ph = 0, out_ph = 0;  

// Rutas de archivos de salida para temperatura y pH
char *file_temp = NULL, *file_ph = NULL;  


//Función que inicializa buffers
void ini_buffers() {
    //Inicializamos los buffers de temperatura y pH
    buffer_temp = (char **)malloc(BUFFER_SIZE * sizeof(char *));
    buffer_ph = (char **)malloc(BUFFER_SIZE * sizeof(char *));
    if (!buffer_temp || !buffer_ph) {
        perror("Error al asignar memoria para los buffers");
        exit(1);
    }

    //Asignamos memoria para cada elemento de los buffers
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer_temp[i] = (char *)malloc(128 * sizeof(char));
        buffer_ph[i] = (char *)malloc(128 * sizeof(char));
        if (!buffer_temp[i] || !buffer_ph[i]) {
            perror("Error al asignar memoria para los buffers");
            exit(1);
        }
    }
}

//Función que libera los buffers
void free_buffers() {
    // Libera la memoria de los buffers y sus elementos
    for (int i = 0; i < BUFFER_SIZE; i++) {
        free(buffer_temp[i]);
        free(buffer_ph[i]);
    }
    free(buffer_temp);
    free(buffer_ph);
}


//Función para recolectar datos desde un pipe nominal
void *recolector(void *param) {


    char *pipe_name = (char *)param;

    //Abrimos el pipe para lectura
    int fd = open(pipe_name, O_RDONLY);  
    if (fd == -1) {

        perror("Error abriendo el pipe en el recolector");
        exit(1);
    }

    char read_buffer[128];

    //Lee datos del pipe y los almacena en el buffer correspondiente
    while (read(fd, read_buffer, sizeof(read_buffer)) > 0) {


        //Eliminamos el salto de línea al final del buffer
        if (read_buffer[strlen(read_buffer) - 1] == '\n') {
            read_buffer[strlen(read_buffer) - 1] = '\0';
        }

        //Dividimos la entrada en tipo de sensor y datos
        char *token = strtok(read_buffer, ":");

        //Tipo de sensor (1: temperatura, 2: pH)
        int sensor_type = atoi(token);  
        token = strtok(NULL, ":");
        char *data = token;

        //Procesa datos de temperatura o pH
        if (sensor_type == 1 || sensor_type == 2) { 
            //Semáforos correspondientes
            sem_t *empty, *full; 

            //Mutex correspondiente
            pthread_mutex_t *mutex;  

            //Buffer correspondiente
            char **buffer;  

            //Índices para el buffer
            int *in;  

            //Si el sensor es temperatura
            if (sensor_type == 1) {
                empty = &empty_temp;
                full = &full_temp;
                mutex = &mutex_temp;
                buffer = buffer_temp;
                in = &in_temp;
            } else {
                //Si el sensor es pH
                empty = &empty_ph;
                full = &full_ph;
                mutex = &mutex_ph;
                buffer = buffer_ph;
                in = &in_ph;
            }

            //Esperamos hasta que haya espacio disponible en el buffer
            sem_wait(empty);

            //Protegemos el acceso al buffer
            pthread_mutex_lock(mutex);  

            //Almacenamos los datos en el buffer con el flag correspondiente
            sprintf(buffer[*in], "%s:%s", (sensor_type == 1) ? TEMPERATURA_FLAG : PH_FLAG, data);

            //Incrementamos el índice de entrada
            *in = (*in + 1) % BUFFER_SIZE;  

            //Liberamos el mutex
            pthread_mutex_unlock(mutex); 

            //Indicamos que hay datos disponibles
            sem_post(full);  
        } else {
            //Avisamos si hay un error
            printf("Error: Medida recibida incorrecta.\n");
        }
    }

    //Esperamos 10 segundos y envía una señal de finalización
    sleep(10);
    sem_post(&empty_temp);
    sem_post(&empty_ph);

    //Cerramos el pipe
    close(fd);  
    return NULL;
}
//Función para procesar datos de pH y escribirlos en un archivo
void *hilo_ph(void *param) {

    //Abrimos el archivo de pH para anexar datos
    FILE *file = fopen(file_ph, "a");  
    if (file == NULL) {
        perror("Error al abrir el archivo de pH");
        exit(1);
    }

    while (1) {
        //Esperamos hasta que haya datos de pH en el buffer
        sem_wait(&full_ph);  
        //Protegemos el acceso al buffer
        pthread_mutex_lock(&mutex_ph);  

        //Compara el contenido de `buffer_ph[out_ph]` con la constante `PH_FLAG` ("PH")
        //`strncmp()` compara hasta una longitud de `strlen(PH_FLAG)`, es decir, 2 caracteres
        //La condición `== 0` verifica si la comparación es igual
        //Verifica si `buffer_ph[out_ph]` corresponde a un dato de pH
        if (strncmp(buffer_ph[out_ph], PH_FLAG, strlen(PH_FLAG)) == 0) {
            //Extraemos los datos
            char *data = buffer_ph[out_ph] + strlen(PH_FLAG) + 1; 

            //Casteamos la cadena a número flotante
            float ph = atof(data);  

            //Obtenemos la hora actual
            time_t now = time(NULL);  

            //Convertimos a una estructura `tm`
            struct tm *tm_info = localtime(&now);  
            char timestamp[20];

            //Formateamos la fecha y hora
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);  

            //Escribimos la fecha, hora y valor de pH en el archivo
            fprintf(file, "[%s] %f\n", timestamp, ph);
            //Vacíamos el buffer inmediatamente
            fflush(file);  

            if (ph < 6 || ph > 8) {
                printf("%sAlerta: PH fuera de rango! %.1f%s\n",RED,ph,RESET);
            }
        }

        //Incrementamos el índice de salida
        out_ph = (out_ph + 1) % BUFFER_SIZE;  

        //Liberamos el mutex
        pthread_mutex_unlock(&mutex_ph);  

        //Indicamos que hay espacio en el buffer
        sem_post(&empty_ph);  
    }

    //Cerramos el archivo
    fclose(file); 
    return NULL;
}

//Función para procesar datos de temperatura y escribirlos en un archivo
void *hilo_temperatura(void *param) {

    //Abrimos el archivo de temperatura para agregar datos
    FILE *file = fopen(file_temp, "a");  
    if (file == NULL) {
        perror("Error al abrir el archivo de temperatura");
        exit(1);
    }

    while (1) {
        //Esperamos hasta que haya datos de temperatura en el buffer
        sem_wait(&full_temp);  
        //Protegemos el acceso al buffer
        pthread_mutex_lock(&mutex_temp);  

        //Compara el contenido de `buffer_temp[out_temp]` con la constante `TEMPERATURA_FLAG` ("TEMPERARTURA")
        //`strncmp()` compara hasta una longitud de `strlen(TEMPERATURA_FLAG)`, es decir, 2 caracteres
        //La condición `== 0` verifica si la comparación es igual
        //Verifica si `buffer_temp[out_temp]` corresponde a un dato de temperatura
        if (strncmp(buffer_temp[out_temp], TEMPERATURA_FLAG, strlen(TEMPERATURA_FLAG)) == 0) {
            //Extraemos los datos
            char *data = buffer_temp[out_temp] + strlen(TEMPERATURA_FLAG) + 1;  
            //Convertimos la cadena a número flotante
            float temp = atof(data); 
            //Obtenemos la hora actual
            time_t now = time(NULL);  
            //Convertimos a una estructura `tm`
            struct tm *tm_info = localtime(&now);  
            char timestamp[20];
            //Formatea la fecha y hora
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);  

            //Escribimos la fecha, hora y temperatura en el archivo
            fprintf(file, "[%s] %f\n", timestamp, temp);
            //Vacíamos el buffer inmediatamente
            fflush(file);  

            if (temp < 20.0 || temp > 31.6) {
                printf("%sAlerta: Temperatura fuera de rango! %.2f%s\n", RED,temp,RESET);
            }
        }
        //Incrementamos el índice de salida
        out_temp = (out_temp + 1) % BUFFER_SIZE;  

        //Liberamos el mutex
        pthread_mutex_unlock(&mutex_temp);  

        //Indicamos que hay espacio en el buffer
        sem_post(&empty_temp);  
    }
    //Cerramos el archivo
    fclose(file);  
    return NULL;
}

//Función principal para inicializar el programa y lanzar los hilos
int main(int argc, char *argv[]) {

    //Verificamos la cantidad de argumentos
    if (argc != 9) {
        fprintf(stderr, "Modo de uso: %s -b <tamaño_buffer> -t <file-temp> -h <file-ph> -p <pipe-nominal>\n", argv[0]);
        exit(1);
    }

    //Procesamos los argumentos de la línea de comandos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0) {
            BUFFER_SIZE = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0) {
            file_temp = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0) {
            file_ph = argv[++i];
        }
    }
    //Imprimimos decoración e información del monitor
    printf("%s┌─────────────────────────────────────────┐%s\n",GREEN,RESET);
    printf("%s│          INFORMACIÓN DEL MONITOR        │%s\n",GREEN,RESET);
    printf("%s├─────────────────────────────────────────┤%s\n",GREEN,RESET);
    printf("%s│ Tamaño del buffer: %d%s\n",GREEN, BUFFER_SIZE,RESET);

    if (!file_temp || !file_ph) {
        fprintf(stderr, "Error al abrir archivo.\n");
        exit(1);
    }

    //Obtenemos la ruta del pipe nominal
    char *pipe_name = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            pipe_name = argv[++i];
        }
    }

    if (!pipe_name) {
        fprintf(stderr, "Error al abrir pipe.\n");
        exit(1);
    }

  //Verificamos si el pipe nominal existe, o lo crea si no es así
  if (access(pipe_name, F_OK) == -1) {
      printf("%s│ Pipe creado: '%s'\n%s", GREEN, pipe_name, RESET);  

      if (mkfifo(pipe_name, 0666) == -1) {
          perror("Error creando el pipe");
          exit(1);
      }
  } else {
      printf("%s│ Pipe existente. No se debe crear.%s\n", GREEN, RESET); 
  }

    //Inicializamos semáforos y mutexes
    printf("%s│ Semáforos inicializados %s\n", GREEN, RESET); 
    sem_init(&empty_temp, 0, BUFFER_SIZE);
    sem_init(&full_temp, 0, 0);
    sem_init(&empty_ph, 0, BUFFER_SIZE);
    sem_init(&full_ph, 0, 0);
    pthread_mutex_init(&mutex_temp, NULL);
    pthread_mutex_init(&mutex_ph, NULL);

    //Inicializa los buffers
    ini_buffers();  
    printf("%s│ Buffers inicializados : 2 %s\n", GREEN, RESET); 
    printf("%s└─────────────────────────────────────────┘%s\n",GREEN,RESET);

    //Creamos los hilos de recolector, pH y temperatura
    pthread_t recolector_thread, ph_thread, temperatura_thread;
    pthread_create(&recolector_thread, NULL, recolector, pipe_name);
    pthread_create(&ph_thread, NULL, hilo_ph, NULL);
    pthread_create(&temperatura_thread, NULL, hilo_temperatura, NULL);

    //Esperamos a que los hilos finalicen
    pthread_join(recolector_thread, NULL);
    pthread_join(ph_thread, NULL);
    pthread_join(temperatura_thread, NULL);

    //Destruye semáforos y mutexes
    sem_destroy(&empty_temp);
    sem_destroy(&full_temp);
    sem_destroy(&empty_ph);
    sem_destroy(&full_ph);
    pthread_mutex_destroy(&mutex_temp);
    pthread_mutex_destroy(&mutex_ph);

    //Libera la memoria de los buffers
    free_buffers();  
    printf("Buffers liberados\n");

    return 0;  // Finaliza la ejecución
}
