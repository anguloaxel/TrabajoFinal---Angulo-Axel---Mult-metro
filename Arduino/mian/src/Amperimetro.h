#ifndef AMPERIMETRO_H
#define AMPERIMETRO_H


#include "SensorBase.h"   // Incluye la clase base abstracta de sensores.


class Amperimetro : public SensorBase {  // Definición de la clase Amperimetro, que hereda de SensorBase.
private:
	int pin;          // Pin analógico donde se lee la salida del sensor de corriente.
	float corriente;  // Variable donde se almacena la corriente medida.
	const float Sensibilidad = 0.100;  // Sensibilidad del sensor usado ( ACS712 20A -> 100 mV/A).
public:
	Amperimetro(int p): pin(p), corriente(0) {}       // Constructor: recibe el pin y pone la corriente inicial en 0.
	void measure() override {             // Método obligatorio de medición que sobrescribe al de la clase base.
		float voltage = analogRead(pin) * 5.0 / 1023.0;     // Convierte la lectura analógica a voltaje (0–1023 ? 0–5 V).
		corriente = (voltage - 2.5) / Sensibilidad;   // Calcula la corriente.
	}
	float getValue() override { return corriente; }   // Devuelve el último valor calculado de corriente.
};


#endif
