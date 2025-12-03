#ifndef TERMOMETRO_H
#define TERMOMETRO_H


#include "SensorBase.h"   // Incluye la clase base SensorBase, de la cual heredará Termometro.
#include "Utils.h"        // Incluye funciones auxiliares (leerPromediado y fmapf).


/*
* Clase: Termometro
* Hereda de: SensorBase
* Descripción:
*   Implementa la lectura de temperatura usando un sensor tipo LM35.
*   El LM35 entrega 10 mV por °C. En ADC de 10 bits (0–1023) con referencia 5V:
*      Voltaje = raw * (5.0 / 1023.0)
*      TempC  = Voltaje * 100  (porque 10 mV por grado ? *100)
*/


class Termometro : public SensorBase {
private:
	int pin;       // Pin analógico donde está conectado el LM35
	float tempC;   // Última temperatura medida en °C
public:
	Termometro(int p): pin(p), tempC(0) {}     // Constructor
	
	// Realiza la medición promediando varias lecturas para reducir ruido
	void measure() override {   
		float raw = leerPromediado(pin, 20);   // Promedia 20 lecturas
		tempC = (raw * 5.0 / 1023.0) * 100.0; // Conversión para LM35
	}
	float getValue() override { return tempC;   // Devuelve la última temperatura medida en °C
	}
};


#endif
