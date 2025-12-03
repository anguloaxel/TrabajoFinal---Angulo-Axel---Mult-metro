#ifndef VOLTIMETRO_H
#define VOLTIMETRO_H


#include "SensorBase.h"  // Incluye la clase base SensorBase, de la cual heredará Voltimetro.
#include "Utils.h"       // Incluye funciones auxiliares (leerPromediado y fmapf).


class Voltimetro : public SensorBase {   // Definición de la clase Voltimetro, que hereda de SensorBase.
private:
	int pin;        // Pin analógico donde se mide el voltaje.
	float voltaje;  // Variable interna donde se guarda el último valor medido.
public:
	Voltimetro(int p): pin(p), voltaje(0) {}      // Constructor: recibe el pin e inicializa la variable voltaje en 0.
	void measure() override {      // Implementación del método obligatorio de medición. Sobrescribe el método virtual puro de SensorBase.
		float raw = leerPromediado(pin, 10);  // Realiza 10 lecturas promediadas del pin analógico.
		voltaje = fmapf(raw, 0, 1023, 0.0, 25.0); // Convierte el valor analógico (0–1023) a voltaje. Escala usada: 0 ? 0 V, 1023 ? 25 V (del diseño original).
	}
	float getValue() override { return voltaje; }  // Retorna el último valor leído del voltímetro.
};


#endif
