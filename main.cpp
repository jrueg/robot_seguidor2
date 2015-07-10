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
	mem_global.angulo = 50;
	mem_global.x = 160;
	mem_global.y = 120;
	mem_global.objetoEncontrado = false;
	mem_global.vel = 0;
	mem_global.salida = true;
	mem_global.remoto = 'x';
	
	//setup GPIO in wiringPi mode
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

	motor_dc motor0(2, 3, 4); //Izquierdo
	motor_dc motor1(5, 6, 7); //Derecho

	sonar sonar(10, 11);


	//VARIABLES GENERALES BUCLE//
	int distanciaseguridad=20, PV1; //cm
	float error, kp, kd, PV, lastError;
	float angulo; //seria mejor "int" pero puede que causara problemas a la hora de las conversiones a float
	float angulobits;
	kp=0.5;
	kd=1;
	lastError=0;

	while (mem_global.salida){
		if (mem_global.remoto != 'x'){
			cout << mem_global.remoto << endl;

			//AQUI EL CODIGO MANUAL
			switch (mem_global.remoto){
				case '1':
					//atras-izquierda
					motor0.velocidad(-80);
					motor1.velocidad(-90);
					break;
				case '2':
					//atras
					motor0.velocidad(-90);
					motor1.velocidad(-90);
					break;
				case '3':
					//atras-derecha
					motor0.velocidad(-90);
					motor1.velocidad(-80);
					break;
				case '4':
					//izquierda
					motor0.velocidad(-90);
					motor1.velocidad(90);
					break;
				case '5':
					//parado
					motor0.velocidad(0);
					motor1.velocidad(0);
					break;
				case '6':
					//derecha
					
					motor0.velocidad(90);
					motor1.velocidad(-90);
					break;
				case '7':
					//adelante-izquierda
					motor0.velocidad(80);
					motor1.velocidad(90);
					break;
				case '8':
					//adelante
					
					motor0.velocidad(90);
					motor1.velocidad(90);
					break;
				case '9':
					//adelante-derecha
					motor0.velocidad(90);
					motor1.velocidad(80);
					break;
			}
			//prueba edicion
		}
		else if (mem_global.objetoEncontrado){
			int distancia = sonar.dist();

			if (distancia == -1){
				cout << "[ERROR] Superado tiempo limite para conexion con sonar" << endl;
			}

			angulo = mem_global.angulo*1.8;
			//cout << "El angulo es: " << angulo << ". La distancia es: " << distancia << endl;
			if (abs(distanciaseguridad - distancia) < 5){
				//robot dentro de la distancia deseada__>PARADO//

				/*CASO SENCILLO-->PARADO SIN GIRAR
				motor0.velocidad(0);
				motor1.velocidad(0);*/

				//Caso de que este dentro de la distacia de seguridad y gire a su alrededor estancionariamente//

				angulobits = angulo*38.88888889; //angulo=angulo en º girado por el servo horizontal
				error = (float)angulobits - 3500;
				int toleranciamin = -(6 * 38.8888); //conversion de ºs a bits
				int toleranciamax = 6 * 38.8888;
				//int velocidadmin=80; //para la PRIMERA VERSION

				if (abs(error) < toleranciamax){
					/*Suponemos que el angulo 0º de la camara mira hacia uno de los lados donde se encuentran las ruedas, situando los 90º hacia el frente del robot.
					La conversion del sistema equivale que 90º son 3500 por lo que le damos un margen de unos 5º para considerarlo mirando hacia el frente del objeto*/

					motor0.velocidad(0);
					motor1.velocidad(0);

				}
				else if (error < toleranciamin){
					//la camara mira a la izquierda por lo que hay que girar a la izquierda estacionariamente (sobre una rueda)
					/*PRIMERA VERSION

					while(error<toleranciamin){

					angulobits = angulo*38.88888889;
					error=(float)angulobits-3500;
					motor0.velocidad(-velocidadmin);
					motor1.velocidad(velocidadmin);//suponemos motor0=izquierda; motor1=derecha. Sobre la velocidad: es aprox. el 80% del valor del motor, lo suficiente para que se mueva
					//con mayores rangos se podria implementar algo parecido al algoritmo de moviemiento aunque aqui resultaria redundante
					}
					*/
					//SEGUNDA VERSION MAS COMPLEJA


					angulobits = angulo*38.88888889;

					error = (float)angulobits - 3500;//recordamos: giro izquierda; valores negativos; giro derecha: valores positivos

					PV = kp * error + kd * (error - lastError);

					lastError = error;

					if (PV > 55){
						PV = 55;
					}
					if (PV < -55){
						PV = -55;
					}
					PV1 = PV * 10 / 55;  //CONVERSION DE BITS A %: MAX.10% PARA PERMITIR EN MINIMO CASO (-10%) EL FUNCIONAMIENTO AL 80%
					motor0.velocidad(90 + PV);
					motor1.velocidad(-(90 + PV));

				}

				else if (error > toleranciamax){

					//IDEM para girar a la derecha estacionariamente
					/*PRIMERA VERSION
					while(error>toleranciamax){
					error=(float)angulobits-3500;
					motor0.velocidad(velocidadmin);
					motor1.velocidad(-velocidadmin);
					}*/
					//SEGUNDA VERSION MAS COMPLEJA


					angulobits = angulo*38.88888889;

					error = (float)angulobits - 3500;//recordamos: giro izquierda; valores negativos; giro derecha: valores positivos

					PV = kp * error + kd * (error - lastError);

					lastError = error;

					if (PV > 55){
						PV = 55;
					}
					if (PV < -55){
						PV = -55;
					}

					PV1 = PV * 10 / 55;  //CONVERSION DE BITS A %: MAX.10% PARA PERMITIR EN MINIMO CASO (-10%) EL FUNCIONAMIENTO AL 80%

					motor0.velocidad(-(90 + PV));
					motor1.velocidad(90 + PV);

				}
			}

			else if (distancia > distanciaseguridad){
				//robot lejos del objeto de referencia__>ADELANTE//
				angulobits = angulo*38.88888889;

				error = (float)angulobits - 3500;//recordamos: giro izquierda; valores negativos; giro derecha: valores positivos

				PV = kp * error + kd * (error - lastError);
				lastError = error;

				if (PV > 55){
					PV = 55;
				}
				if (PV < -55){
					PV = -55;
				}
				PV1 = PV * 10 / 55;
				motor0.velocidad(90 - PV1);
				motor1.velocidad(90 + PV1);
			}
			else{
				//robot demasiado cerca__>ATRAS//

				angulobits = angulo*38.88888889;

				error = (float)angulobits - 3500;

				PV = kp * error + kd * (error - lastError);
				lastError = error;

				if (PV > 55){
					PV = 55;
				}
				if (PV < -55){
					PV = -55;
				}
				PV1 = PV * 10 / 55;
				motor1.velocidad(-(90 - PV1));
				motor0.velocidad(-(90 + PV1));
			}
		}
		else{
			motor0.velocidad(0);
			motor1.velocidad(0);
		}
	}
	

	cout << "Terminando programa. Esperando threads secundarios..." << endl;

	th_seguimiento.join();
	th_bluecom.join();

	cout << "FIN" << endl;

	return 0;
}
