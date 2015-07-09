/*
seguimiento.h - Encabezado de las funciones del seguimiento

Jesus Rueda Gonzalez - 19/05/2015

UPCT
*/

#ifndef _FUNC_MAIN_H
#define _FUNC_MAIN_H

struct mem_global {
	int H_MIN;
	int H_MAX;
	int S_MIN;
	int S_MAX;
	int V_MIN;
	int V_MAX;
	double x;
	double y;
	double vel;
	bool salida;
	bool objetoEncontrado;
};

//Para seguimiento
void seguimiento(struct mem_global *mem_global);

//Para comunicaciones
void bluecom(struct mem_global *mem_global);

#endif
