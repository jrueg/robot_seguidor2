/*
control.h - Encabezado de las funciones de control

Jesus Rueda Gonzalez - 19/05/2015

UPCT
*/

#ifndef _FUNC_CONTROL_H
#define _FUNC_CONTROL_H

class controlador_p{
public:
	controlador_p(double _P, double _lim_sup, double _lim_inf, double _hist);
	int setpoint(double _ref);
	int feedback(double _realim);
	double calculo();
	double calculo_realim(double _realim);
	double calculo_ref(double _ref);
	double calculo(double _realim, double _ref);
	int redefine(double _P, double _lim_sup, double _lim_inf, double _hist);

private:
	double P;
	double ref;
	double realim;
	double lim_inf;
	double lim_sup;
	double hist;
};

class motor_dc {
public:
	motor_dc(unsigned char _EN, unsigned char _C1, unsigned char _C2);
	~motor_dc();
	void velocidad(int _vel);
private:
	unsigned char Pin_EN;
	unsigned char Pin_C1;
	unsigned char Pin_C2;
	int vel;
};

void servoBlaster(unsigned char pin, int vel);

class sonar{
public:
	sonar(unsigned char _echo, unsigned char _trig);
	int dist();
private:
	unsigned char echo;
	unsigned char trig;
};

#endif