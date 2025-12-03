#ifndef INDUCTOMETRO_H
#define INDUCTOMETRO_H


#include "SensorBase.h"  // Incluye la clase base abstracta de sensores


class Inductometro : public SensorBase {  // Declara la clase Inductometro heredando de SensorBase
private:
	int pinMedida;     // Pin donde se mide el pulso resonante
	int pinPulso;      // Pin que genera el pulso de excitación
	double pulse;      // Variable para almacenar el ancho de pulso medido
	double inductance; // Variable para almacenar la inductancia calculada

	
public:
	// Constructor con pines por defecto (4 medición, 3 pulso)
	Inductometro(int pm = 4, int pp = 3): pinMedida(pm), pinPulso(pp), pulse(0), inductance(0) {
		pinMode(pinPulso, OUTPUT);  // Configura el pin de pulso como salida
		pinMode(pinMedida, INPUT);  // Configura el pin de medición como entrada
	}
	
	double medirPulsoPromedio(int muestras = 5) {    // Función que mide el pulso varias veces y promedia
		
		double suma = 0;       // Variable acumuladora para sumar las lecturas
		
		for (int i = 0; i < muestras; i++) {     // Bucle para realizar 'muestras' mediciones
			
			digitalWrite(pinPulso, HIGH);     // Genera un pulso alto para excitar el circuito LC
			delay(5);         // Mantiene el pulso durante 5 ms
			
			digitalWrite(pinPulso, LOW);      // Apaga el pulso para liberar el circuito
			delayMicroseconds(100);      // Espera 100 microsegundos antes de medir
			
			suma += pulseIn(pinMedida, HIGH, 5000);    // Mide la duración del pulso resonante (timeout 5 ms)
		}
		return suma / muestras;    // Retorna el promedio de las mediciones
	}
	
	void measure() override {    // Implementación del método measure() obligatorio en SensorBase
		
		pulse = medirPulsoPromedio();     // Obtiene el pulso promedio y lo guarda en 'pulse'
		
		if (pulse > 0.1) {   // Verifica que haya una medición válida (evita valores nulos)
			double capacitance = 1.E-7;    // Capacitancia fija del circuito (100 nF = 1×10?7 F)
			double frequency = 1.E6 / (2.0 * pulse);     // Convierte el ancho de pulso en una frecuencia aproximada
			inductance = 1.0 / (capacitance * frequency * frequency * 4.0 * 3.14159 * 3.14159);   // Aplica la fórmula L = 1 / (C*(2pf)²)
			inductance *= 1E6;   // Convierte la inductancia de Henrios a microHenrios (µH)
			inductance = inductance * 0.61664; // Aplica un factor de calibración experimental
		} else {               // Si no hay pulso válido
			inductance = 0;    // Fija la inductancia en cero
		}
	}
	
	float getValue() override {    // Implementa el método getValue() de SensorBase
		return (float)inductance;  // Devuelve la inductancia actual como float
	}
	
	String getDisplayString() {     // Método auxiliar para formatear el valor para pantalla
		char buf[32];        // Buffer de caracteres para sprintf
		sprintf(buf, "%5.2f uH", (float)inductance);     // Convierte la inductancia en string formateado
		return String(buf);      // Devuelve el texto formateado como String
	}
};


#endif
