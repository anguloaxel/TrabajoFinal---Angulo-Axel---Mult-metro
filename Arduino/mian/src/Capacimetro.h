#ifndef CAPACIMETRO_H
#define CAPACIMETRO_H

#include "SensorBase.h"    // Clase base de sensores
#include <Capacitor.h>     // Librería para medir capacitancias pequeñas

// --- Constantes y pines usados ---
#define resistencia_H  10035.00F      // Resistencia usada en carga lenta (alta)
#define resistencia_L  275.00F        // Resistencia usada en carga rápida (baja)

#define CapIN_H     A2                   // Entrada analógica de alta resistencia
#define CapOUT      A1                    // Salida de señal para carga/descarga
#define CapIN_L     A0                   // Entrada analógica de baja resistencia
#define cargaPin     9                   // Pin para cargar el capacitor
#define descargaPin  8                // Pin para descargarlo

class Capacimetro {
private:
	
	// --- Objeto de librería para medir pF ---
	Capacitor pFcap = Capacitor(A1, A0);  // Inicializa medición pF entre A1 y A0
	
	// --- Variables de tiempo ---
	unsigned long iniTime = 0;       // Tiempo al iniciar la medición
	unsigned long endTime = 0;       // Duración de la carga hasta 63% (1 tau)
	
	// --- Variables para medir ESR y referencia ADC ---
	int ADCref = 0;                  // Voltaje de referencia interno medido
	int sampleESR = 0;               // Lectura cruda para ESR
	float milliVolts = 0;            // Conversión de lectura ESR a milivoltios
	float esr = 0;                   // Valor final de ESR en ohms
	const int Repe = 5;              // Cantidad de mediciones para promediar
	
	// --- Offsets de calibración ---
	float Off_pF_Hr = 0;             // Offset carga rápida
	float Off_pF_H = 0;              // Offset carga lenta
	float Off_pF_Low = 0;            // Offset para pF medidos
	float Off_GND = 0;               // Offset del nivel GND
	
	// --- Resultado final ---
	float valor = 0;                 // Valor numérico medido
	String unidad = "";              // Unidad (uF, nF, pF)
	String tipo = "";                // Tipo de capacitor detectado
	
public:

	Capacimetro() {}                 // Constructor vacío
	
	void begin() {
		pFcap.Calibrate(41.95, 36.00);  // Calibración de la librería para pF
		pinMode(CapOUT, OUTPUT);        // Configura salida CapOUT
		pinMode(CapIN_L, OUTPUT);       // Configura entrada baja como salida inicial
		calibrado();                    // Ejecuta rutina de calibración completa
	}
	
	void beginCalibrationOnly() { 
		pFcap.Calibrate(41.95, 36.00);  // Solo calibra librería pF
	}
	
	void calibrado() {
		descargaCap();                           // Asegura capacitor totalmente descargado
		cargaCap_Slow();                         // Realiza carga lenta
		Off_pF_H = ((float)endTime / resistencia_H) * 1000000;  // Calcula offset
		
		descargaCap();                           // Descarga de nuevo
		cargaCap_Fast();                         // Realiza carga rápida
		Off_pF_Hr = ((float)endTime / resistencia_L) * 1000000; // Offset carga rápida
		
		midePF();                                // Mide valor en pF
		Off_pF_Low = valor;                      // Guarda offset pF
		
		medidaADC();                             // Mide referencia ADC
	}
	
	void descargaCap() {
		analogReference(DEFAULT);                // Usa referencia ADC por defecto
		pinMode(CapIN_H, INPUT);                 // Define pin de lectura
		
		pinMode(cargaPin, OUTPUT);               
		digitalWrite(cargaPin, LOW);             // Fuerza descarga por cargaPin
		
		pinMode(descargaPin, OUTPUT);
		digitalWrite(descargaPin, LOW);          // Fuerza descarga total
		
		while (analogRead(CapIN_H) > 0) {}       // Espera a que llegue a 0
		
		pinMode(descargaPin, INPUT);             // Libera pines
		pinMode(cargaPin, INPUT);
	}
	
	void cargaCap_Slow() {
		pinMode(cargaPin, OUTPUT);               // Pin de carga
		digitalWrite(cargaPin, HIGH);            // Comienza carga lenta
		
		iniTime = micros();                      // Marca inicio
		while (analogRead(CapIN_H) < 645) {}     // Espera llegar a 63% de 1023 (˜645)
		
		endTime = micros() - iniTime;            // Calcula tiempo de carga
	}
	
	void cargaCap_Fast() {
		pinMode(descargaPin, OUTPUT);            // Carga por descargaPin (resistencia baja)
		digitalWrite(descargaPin, HIGH);         // Carga rápida
		
		iniTime = micros();                      // Marca inicio
		while (analogRead(CapIN_H) < 645) {}     // Espera llegar al 63%
		
		endTime = micros() - iniTime;            // Tiempo final
	}
	
	
	int refADC() {
		long result;
		
		ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); // Selecciona referencia interna 1.1V
		delay(2);                                               // Estabiliza
		ADCSRA |= _BV(ADSC);                                    // Inicia conversión
		
		while (bit_is_set(ADCSRA, ADSC));                       // Espera conversión
		
		result = ADCL;                                          // Lee parte baja
		result |= ADCH << 8;                                    // Completa valor ADC
		
		result = 1125300L / result;                             // Calculo voltaje real Vcc
		
		return result;                                          // Devuelve referencia real
	}
	
	void medidaADC() {
		ADCref = refADC();             // Guarda referencia ADC medida
	}
	
	void mideESR() {
		descargaCap();                 // Descarga completamente
		
		digitalWrite(CapOUT, LOW);     // Lleva salida a 0
		pinMode(descargaPin, OUTPUT);
		digitalWrite(descargaPin, LOW);
		
		delayMicroseconds(100);        // Espera mínima
		digitalWrite(descargaPin, HIGH);  // Aplica pulso
		delayMicroseconds(5);
		
		sampleESR = analogRead(CapIN_H); // Lee subida instantánea
		Off_GND = analogRead(CapOUT);    // Offset tierra
		sampleESR -= Off_GND;            // Compensa
		
		descargaCap();                   // Descarga nuevamente
		
		milliVolts = (sampleESR * (float)ADCref) / 1023; // Convertir a mV
		int R_GND = resistencia_L / 1023 * Off_GND;      // Efecto del offset GND
		
		esr = (resistencia_L + R_GND) / (((float)ADCref / milliVolts) - 1); // Cálculo ESR
		esr -= 0.9;                             // Compensación empírica
		if (esr < 0) esr = 0;                   // No puede ser negativo
	}
	
	void midePF() {
		descargaCap();                 // Descarga capacitor
		float valorMedio = 0;
		
		for (int i = 0; i < Repe; i++) {   // Repite medición Repe veces
			valor = pFcap.Measure();       // Medición usando librería
			valorMedio += valor;           // Suma
			descargaCap();                 // Descarga nuevamente
		}
		
		valor = valorMedio / Repe;         // Promedio
	}
	
	void measure() {
		
		descargaCap();                     // Asegura capacitor descargado
		pinMode(descargaPin, OUTPUT);
		digitalWrite(descargaPin, HIGH);   // Precarga
		delay(1);
		
		unsigned int muestra1 = analogRead(CapIN_H);  // Primera lectura
		delay(100);
		unsigned int muestra2 = analogRead(CapIN_H);  // Segunda lectura
		unsigned int cambio = muestra2 - muestra1;    // Cambio en tensión
		
		if (muestra2 < 1000 && cambio < 30) { // Condición: capacitor muy grande
			tipo = "[Test]";                // Se detecta como capacitor alto
		}
		else {
			descargaCap();
			cargaCap_Fast();                // Intento con resistencia baja
			float medidaLocal = ((float)endTime / resistencia_L) - (Off_pF_Hr / 1e6);
			
			if (medidaLocal < 80) {         // Si es menor a 80uF, usar método lento
				tipo = " <80uF";
				descargaCap();
				cargaCap_Slow();
				medidaLocal = ((float)endTime / resistencia_H) - (Off_pF_H / 1e6);
			}
			else {
				tipo = " >80uF";            // Capacitor grande
			}
			
			if (medidaLocal > 1) {          // Si es 1 uF o más
				valor = medidaLocal;
				unidad = " uF";
			}
			else if (medidaLocal > 0.05) {  // Entre 0.05 y 1 uF ? nF
				valor = medidaLocal * 1000;
				unidad = " nF";
			}
			else {                          // Muy chico ? usar método pF
				midePF();
				valor = valor - Off_pF_Low;
				unidad = " pF";
			}
		}
	}
	
	float getValue() { return valor; }    // Devuelve valor numérico
	
	String getDisplayString() {
		char buf[32];
		char num[16];
		

		// Convertir float en texto 
		dtostrf(valor, 0, 2, num);     // 2 decimales, sin espacios extra
		
		// Unidades ASCII 
		snprintf(buf, sizeof(buf), "%s %s", num, unidad.c_str());

		
		return String(buf);
	}
};

#endif
