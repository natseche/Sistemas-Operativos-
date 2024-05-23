

#**************************************************************
#               Pontificia Universidad Javeriana
#     Autor: Natalia Echeverry Salcedo, Maria Paula Rodriguez, Daniel Carvajal y Juan Pablo Cañón Contreras
#     Fecha: Mayo 2024
#     Materia: Sistemas Operativos
#     Tema: Taller de Evaluación de Rendimiento
#     Fichero: script automatización ejecución por lotes 
#     Objetivo: Documentar el código y modificar los valores
#                para la experimentación y salida de datos
#****************************************************************

use strict;
use warnings;
use File::Path qw(make_path);

# Obtiene el directorio de trabajo actual y lo asigna a $Path
my $Path = `pwd`;

# Elimina cualquier salto de línea al final de la salida del comando 'pwd'
chomp($Path);  

# Define la carpeta donde se guardarán los resultados
my $result_dir = "$Path/resultados";

# Crea la carpeta si no existe
make_path($result_dir) unless -d $result_dir;

# Define el nombre de los ejecutables que se usarán en las pruebas
my @Ejecutables = ("mm_clasico", "mm_transpuesta");

# Define los tamaños de matrices para las pruebas en un arreglo
my @Size_Matriz = ("1024");

# Define los números de hilos que se usarán en las pruebas en otro arreglo
my @Num_Hilos = (1, 2, 4);

# Establece el número de repeticiones que se realizarán para cada configuración
my $Repeticiones = 10;

# Ciclo anidado que itera sobre cada ejecutable, tamaño de matriz y número de hilos
foreach my $exe (@Ejecutables) {
    foreach my $size (@Size_Matriz) {
        foreach my $hilo (@Num_Hilos) {

            # Asigna el nombre concatenando la ruta, nombre del ejecutable, tamaño e hilos
            my $file = "$result_dir/${exe}_$size-Hilos-$hilo.dat";

            # Abrir el archivo y mostrar mensaje de error en caso de que no se pueda
            open(my $fh, '>', $file) or die "No se pudo abrir el archivo '$file' $!";

            # Ciclo que itera por la cantidad de repeticiones
            for (my $i = 0; $i < $Repeticiones; $i++) {
                # Ejecutar linea de comando y sino se puede decir mensaje de error
                my $command = "$Path/$exe $size $hilo";
                print "Ejecutando comando: $command\n";
                open(my $cmd, "-|", $command) or die "No se pudo ejecutar el comando '$command': $!";
                while (my $line = <$cmd>) {
                    print $fh $line;  # Imprime la linea en el archivo
                }
                close $cmd; #Finaliza el uso de la consola
            }

            close $fh; #Cierra el archivo y finaliza el script
        }
    }
}
