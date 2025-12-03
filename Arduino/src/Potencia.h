#ifndef POTENCIA_H
#define POTENCIA_H


#include "SensorBase.h"


class Potencia : public SensorBase { // Definición de la clase Potencia, que también es un "sensor lógico". No mide directamente, sino que calcula P = V * I.
private:
	Voltimetro* v;     // Puntero al objeto voltímetro (fuente de voltaje).
	Amperimetro* a;    // Puntero al objeto amperímetro (fuente de corriente).
	float potencia;    // Variable donde se guarda la potencia calculada.
public:
	
	// Constructor: recibe opcionalmente punteros a los sensores.
	// Si no se pasan, se inicializan como NULL.
	Potencia(Voltimetro* vv = NULL, Amperimetro* aa = NULL): v(vv), a(aa), potencia(0) {}
	
	// Método para asignar o reasignar sensores después de construido el objeto.
	void setSensors(Voltimetro* vv, Amperimetro* aa) { 
		v = vv; 
		a = aa; 
	}
	
	// Método que realiza el cálculo de potencia.
	// Sólo calcula si ambos sensores existen (no son NULL).
	void compute() { 
		if (v && a) 
			potencia = v->getValue() * a->getValue(); 
	}
	
	// Sobrescribe measure(), que simplemente llama a compute().
	void measure() override { compute(); }
	
	// Devuelve la potencia calculada.
	float getValue() override { return potencia; }
};


#endif
