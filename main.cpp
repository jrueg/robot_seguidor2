#include <sstream>
#include <string>
#include <iostream>
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <thread>
#include <raspicam/raspicam_cv.h>
#include <time.h>
#include <stdint.h> //uint8_t definitions
#include <stdlib.h> //for exit(int);
#include <wiringPi.h>
#include <wiringSerial.h>
#include <errno.h>
#include "control.h"
#include "main.h"

using namespace std;

int main(int argc, char* argv[])
{
	// Valores iniciales de la memoria compartida
	struct mem_global mem_global;
	
	mem_global.H_MIN = 153;
	mem_global.H_MAX = 231;
	mem_global.S_MIN = 102;
	mem_global.S_MAX = 256;
	mem_global.V_MIN = 0;
	mem_global.V_MAX = 256;
	mem_global.x = 160;
	mem_global.y = 120;
	mem_global.objetoEncontrado = false;
	mem_global.vel = 0;
	mem_global.salida = true;
	
	//Iniciar WirintPi con numeracion de pines de WiringPi
	if (wiringPiSetup () == -1){
		cout << "Imposible iniciar wiringPi: " << strerror (errno) << endl;
		exit(1); //error
	}	

	//Lanza thread seguimiento
	std::thread th_seguimiento(seguimiento, &mem_global);
	
	//Lanza thread comunicaciones
	std::thread th_bluecom(bluecom, &mem_global);

	int pos = 5;
	int vel = -100;

	motor_dc motor0(2, 3, 4);
	motor_dc motor1(5, 6, 7);

	sonar sonar(10, 11);

	while (mem_global.salida){
		//cout << "Desde thread principal: x = " << mem_global.x << " y = " << mem_global.y << endl;
		/*cout << "Servo 0: " << pos << " Servo 1: " << 100 - pos << endl;
		servoBlaster(0, pos);
		servoBlaster(1, 100 - pos);
		pos++;
		if (pos == 95) pos = 5;
		cout << "Motor 0: " << vel << endl;
		motor0.velocidad(vel);
		motor1.velocidad(vel);
		vel += 10;
		if (vel > 100) vel = -100;
		*/
		//cout << "Distancia: " << sonar.dist() << endl;
		delay(100);
	}

	cout << "Terminando programa. Esperando threads secundarios..." << endl;

	th_seguimiento.join();
	th_bluecom.join();

	cout << "FIN" << endl;

	return 0;
}
