# Variables para los nombres de los ejecutables y archivos fuente
CC = gcc
CFLAGS = -Wall
MONITOR_SRC = monitor.c
SENSOR_SRC = sensor.c
MONITOR_BIN = monitor
SENSOR_BIN = sensor

# Compilar monitor
$(MONITOR_BIN): $(MONITOR_SRC)
		$(CC) $(CFLAGS) $(MONITOR_SRC) -o $(MONITOR_BIN)

# Compilar sensor
$(SENSOR_BIN): $(SENSOR_SRC)
		$(CC) $(CFLAGS) $(SENSOR_SRC) -o $(SENSOR_BIN)

# Ejecutar monitor y sensor de temperatura
run_temp: $(MONITOR_BIN) $(SENSOR_BIN)
		./$(MONITOR_BIN) -b 100 -t file-temp.txt -h file-ph.txt -p pipe_nominal & \
		sleep 1 && ./$(SENSOR_BIN) -s 1 -t 1 -f temperatura.txt -p pipe_nominal

# Ejecutar monitor y sensor de pH
run_ph: $(MONITOR_BIN) $(SENSOR_BIN)
		./$(MONITOR_BIN) -b 100 -t file-temp.txt -h file-ph.txt -p pipe_nominal & \
		sleep 1 && ./$(SENSOR_BIN) -s 2 -t 1 -f phdata.txt -p pipe_nominal

# Limpiar archivos generados
clean:
		rm -f $(MONITOR_BIN) $(SENSOR_BIN) file-temp.txt file-ph.txt temperatura.txt phdata.txt

# Reglas para compilar todo y ejecutar casos específicos
all: $(MONITOR_BIN) $(SENSOR_BIN)

# Definir una regla por defecto para evitar compilaciones innecesarias
.PHONY: clean all run_temp run_ph
