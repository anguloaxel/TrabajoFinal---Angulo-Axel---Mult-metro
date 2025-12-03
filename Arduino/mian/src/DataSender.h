#ifndef DATASENDER_H
#define DATASENDER_H

#include <Arduino.h>

// -----------------------------------------------------------------------------
//  Clase DataSender: se encarga de construir un string con los valores activos
//  y enviarlo mediante Serial en un único println() por ciclo.
// -----------------------------------------------------------------------------
class DataSender {
public:
	
	// Constructor vacío (no hace nada especial)
	DataSender() {}
	
	// Inicializa el puerto serie a la velocidad dada
	void begin(long baudRate){
		Serial.begin(baudRate);
	}
		
		// ---------------------------------------------------------------------
		// Método send(): recibe los estados y valores y arma un string final
		// Ejemplo salida: "V12.03,T25.88,P40.21,"
		// ---------------------------------------------------------------------
		void send(
				  bool estVolt, float v,        // Estado y valor Voltaje
				  bool estAmp,  float a,        // Estado y valor Corriente
				  bool estPot,  float p,        // Estado y valor Potencia
				  bool estTemp, float t,        // Estado y valor Temperatura
				  bool estInd,  float l,        // Estado y valor Inductancia
				  bool estCap,  String cValue   // Estado y cadena de Capacitancia
				  ){
			// String donde se concatenará todo
			String msg = "";
			
			// Si la opción Voltaje está activa, agregar al string
			if (estVolt) {
				msg += "V";       // Prefijo V
				msg += String(v); // Valor numérico
				msg += ",";       // Separador
			}
			
			// Si la opción Corriente está activa
			if (estAmp) {
				msg += "A";
				msg += String(a);
				msg += ",";
			}
			
			// Si la opción Potencia está activa
			if (estPot) {
				msg += "P";
				msg += String(p);
				msg += ",";
			}
			
			// Si la opción Temperatura está activa
			if (estTemp) {
				msg += "T";
				msg += String(t);
				msg += ",";
			}
			
			// Si la opción Inductancia está activa
			if (estInd) {
				msg += "I";
				msg += String(l);
				msg += ",";
			}
			
			// Si la opción Capacitancia está activa
			if (estCap) {
				msg += "C";
				msg += cValue;     // text string desde tu clase cap
				msg += ",";
			}
			
			// Si hay algo para enviar, imprimir una sola línea
			if (msg.length() > 0) {
				Serial.println(msg);
			}
		}
};

#endif
