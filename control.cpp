/*
control.cpp - Implementación de las funciones de control

Jesus Rueda Gonzalez - 19/05/2015

UPCT
*/

#include <string>
#include <iostream>
#include <wiringPi.h>
#include <softPwm.h>
#include <cmath>
#include "control.h"

controlador_p::controlador_p(double _P, double _lim_sup, double _lim_inf, double _hist){
	P = _P;
	ref = 0;
	realim = 0;
	hist = _hist;
	lim_sup = _lim_sup;
	lim_inf = _lim_inf;
}

int controlador_p::setpoint(double _ref){
	ref = _ref;
	return(0);
}

int controlador_p::feedback(double _realim){
	realim = _realim;
	return(0);
}

double controlador_p::calculo(){
	double error, salida;

	error = ref - realim;

	if (std::abs(error) < hist) return(0);

	salida = error*P;

	if (salida > lim_sup)
	{
		salida = lim_sup;
	}
	else if (salida < lim_inf)
	{
		salida = lim_inf;
	}

	return(salida);
}

double controlador_p::calculo_realim(double _realim){
	double error, salida;

	realim = _realim;

	error = ref - realim;

	if (std::abs(error) < hist) return(0);

	salida = error*P;

	//std::cout << "Error: " << error << " salida = " << salida << std::endl;

	if (salida > lim_sup)
	{
		salida = lim_sup;
	}
	else if (salida < lim_inf)
	{
		salida = lim_inf;
	}
	
	return(salida);
}

double controlador_p::calculo_ref(double _ref){
	double error, salida;

	ref = _ref;

	error = ref - realim;

	if (std::abs(error) < hist) return(0);

	salida = error*P;

	if (salida > lim_sup)
	{
		salida = lim_sup;
	}
	else if (salida < lim_inf)
	{
		salida = lim_inf;
	}

	return(salida);
}

double controlador_p::calculo(double _ref, double _realim){
	double error, salida;

	realim = _realim;
	ref = _ref;

	error = ref - realim;

	if (std::abs(error) < hist) return(0);

	salida = error*P;

	if (salida > lim_sup)
	{
		salida = lim_sup;
	}
	else if (salida < lim_inf)
	{
		salida = lim_inf;
	}

	return(salida);
}


int controlador_p::redefine(double _P, double _lim_sup, double _lim_inf, double _hist){
	P = _P;
	ref = 0;
	realim = 0;
	hist = _hist;
	lim_sup = _lim_sup;
	lim_inf = _lim_inf;
	return(0);
}

motor_dc::motor_dc(unsigned char _Pin_EN, unsigned char _Pin_C1, unsigned char _Pin_C2){
	Pin_EN = _Pin_EN;
	Pin_C1 = _Pin_C1;
	Pin_C2 = _Pin_C2;
	vel = 0;

	pinMode(Pin_C1, OUTPUT);
	pinMode(Pin_C2, OUTPUT);
	softPwmCreate(_Pin_EN, 0, 100);

	digitalWrite(Pin_C1, 0);
	digitalWrite(Pin_C2, 0);
	softPwmWrite(Pin_EN, vel);
}

motor_dc::~motor_dc(){
	digitalWrite(Pin_C1, 0);
	digitalWrite(Pin_C2, 0);
	vel = 0;
	softPwmWrite(Pin_EN, vel);
}

void motor_dc::velocidad(int _vel){
	int uvel;

	if (_vel == 0){
		digitalWrite(Pin_C1, 0);
		digitalWrite(Pin_C2, 0);
	}

	if (_vel < 0 && vel >= 0){
		digitalWrite(Pin_C1, 0);
		digitalWrite(Pin_C2, 0);
		digitalWrite(Pin_C2, 1);
	}

	if (_vel > 0 && vel <= 0){
		digitalWrite(Pin_C2, 0);
		digitalWrite(Pin_C1, 0);
		digitalWrite(Pin_C1, 1);
	}

	vel = _vel;

	uvel = std::abs(vel);

	if (uvel < 0) uvel = 0;
	if (uvel > 100) uvel = 100;

	softPwmWrite(Pin_EN, uvel);

}

void servoBlaster(unsigned char pin, int vel){
	// Correccion de la velocidad en limites
	if (vel < 0) vel = 0;
	if (vel > 100) vel = 100;
	// Envio del comando en porcentaje por consola
	std::string servoPos = "echo " + std::to_string(pin) + "=" + std::to_string(vel) + "% > /dev/servoblaster";
	system(servoPos.c_str());
}

sonar::sonar(unsigned char _echo, unsigned char _trig){
	echo = _echo;
	trig = _trig;

	pinMode(echo, INPUT);
	pinMode(trig, OUTPUT);
	digitalWrite(trig, 0);
}

int sonar::dist(){
	//Mandar pulso
	digitalWrite(trig, 1);
	delayMicroseconds(20);
	digitalWrite(trig, 0);

	//Esperar al eco
	while (digitalRead(echo) == 0);

	//Esperar al fin del eco
	long tiempo_inicial = micros();
	while (digitalRead(echo) == 1);
	long tiempo_vuelo = micros() - tiempo_inicial;

	//Distancia (en centimetros)
	return tiempo_vuelo / 58;
}