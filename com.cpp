/*
com.cpp - Implementación de las funciones de comunicacion

Jesus Rueda Gonzalez - 19/05/2015

UPCT
*/

#include <sstream>
#include <iostream>
#include <stdint.h> //uint8_t definitions
#include <stdlib.h> //para exit(int);
#include <string.h> //para errno
#include <errno.h> 
#include <string>
#include <wiringPi.h>
#include <wiringSerial.h>
//#include "control.h"
#include "main.h"

using namespace std;

void bluecom(struct mem_global *mem_global){
	

	// FTDI_PROGRAMMER "/dev/ttyUSB0"
	// HARDWARE_UART "/dev/ttyAMA0"
	char device[]= "/dev/ttyAMA0";

	int fd;
	unsigned long baud = 9600;
	unsigned long time=0;
	unsigned long time2 = 0;

	cout << "Iniciando comunicacion" << endl;
	fflush(stdout);
	 
	//Abrir puerto serie
	if ((fd = serialOpen (device, baud)) < 0){
		cout << "No se puede abrir el puerto serie: " << strerror (errno) << endl;
	    exit(1); //error
	}
	
	while((*mem_global).salida){

		if(millis()-time>=100){
			//Escribir por bluetooth los datos de posicion del objeto cada segundo
			(*mem_global).remoto = 'x';
		}
	 
		// Tratamiento de mensajes recibidos
		if(serialDataAvail (fd)){
		(*mem_global).remoto = serialGetchar (fd);
			if ((*mem_global).remoto == -1){
				(*mem_global).remoto = 'x';
				cout << "Error al leer carácter: Sin datos disponibles!" << endl;
			}
			else{
				//cout << newChar;
				//Una vez recibido el caracter se puede identificar para generar acciones
				
				if ((*mem_global).remoto == ':'){ //Cierre de programa
					cout << "Recibido caracter de terminacion de programa." << endl;
					(*mem_global).salida = false;
					serialPuts(fd, "Finalizando programa...");
				}
				fflush(stdout);
				time = millis();
			}
  		}	

	}
	
	cout << "Cerrando puerto serie" << endl;
	serialClose(fd);

}
