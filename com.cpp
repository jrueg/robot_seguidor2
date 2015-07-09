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

		if(millis()-time>=1000){
			//Escribir por bluetooth los datos de posicion del objeto cada segundo
			std::string mandar = "x = " + std::to_string((*mem_global).x) + " y = " + std::to_string((*mem_global).y) + "\n";
			serialPuts (fd, mandar.c_str());
			time=millis();
		}
	 
		// Tratamiento de mensajes recibidos
		if(serialDataAvail (fd)){
		char newChar = serialGetchar (fd);
			if(newChar == -1){
				cout << "Error al leer carácter: Sin datos disponibles!" << endl;
			}
			else{
				cout << newChar;
				//Una vez recibido el caracter se puede identificar para generar acciones
				if (newChar == ':'){ //Cierre de programa
					cout << "Recibido caracter de terminacion de programa." << endl;
					(*mem_global).salida = false;
					serialPuts(fd, "Finalizando programa...");
				}
				fflush(stdout);
			}
  		}	
		//Asegurar que el programa cierra si pasa demasiado tiempo
		if (millis() - time2 >= 1000000){
			cout << "Superado el tiempo limite de prueba." << endl;
			(*mem_global).salida = false;
		}
	}
	
	cout << "Cerrando puerto serie" << endl;
	serialClose(fd);

}
