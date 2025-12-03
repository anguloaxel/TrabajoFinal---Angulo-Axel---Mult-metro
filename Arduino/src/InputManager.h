#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <Arduino.h>

class InputManager {   // Clase que maneja el botón (con debounce no bloqueante) y comandos por Serial.
	private:
		int botonPin;                       // Pin donde está conectado el botón
		unsigned long lastDebounce;         // Tiempo del último cambio detectado (millis)
		bool lastState;                     // Último estado leído del pin (HIGH/LOW)
		const unsigned long debounceDelay = 300; // Tiempo de debounce en ms
	
	public:
		// Constructor: guarda pin y configura INPUT_PULLUP
		InputManager(int bp) : botonPin(bp), lastDebounce(0), lastState(HIGH)
		{
			pinMode(botonPin, INPUT_PULLUP); // Usamos pull-up interno
		}
		
		// Método de inicio (vacío por ahora)
		void begin() { }
		
		
		//----------------------------------------------------------
		//  FUNCION: update()
		//  Detecta FLANCO de bajada del botón y cambia la opción.
		//----------------------------------------------------------
		void update(int &opcion) {
			bool lectura = digitalRead(botonPin);
			
			// Detectar flanco HIGH -> LOW
			if (lectura != lastState && lectura == LOW) {
				if (millis() - lastDebounce > debounceDelay) {
					
					// --- Cambiar a la siguiente opción ---
					opcion++;
					if (opcion > 6) opcion = 1;
					
					lastDebounce = millis();
				}
			}
			
			lastState = lectura;
		}
		
		// Lee el puerto serie y actualiza estados/opcion
		void checkSerialCommands(bool &estadoVolt, bool &estadoAmp, bool &estadoPot, bool &estadoTemp, bool &estadoInd, bool &estadoCap, int &opcion)
		{
			String codigo = "";
			while (Serial.available() > 0) {
				char c = Serial.read();
				if (c != '\n' && c != '\r') codigo += c;
			}
			if (codigo.length() == 0) return;
			if (codigo == "V1") { estadoVolt = true; opcion = 1; }
			else if (codigo == "V0") { estadoVolt = false; }
			else if (codigo == "A1") { estadoAmp = true; opcion = 2; }
			else if (codigo == "A0") { estadoAmp = false; }
			else if (codigo == "P1") { estadoPot = true; opcion = 3; }
			else if (codigo == "P0") { estadoPot = false; }
			else if (codigo == "T1") { estadoTemp = true; opcion = 4; }
			else if (codigo == "T0") { estadoTemp = false; }
			else if (codigo == "I1") { estadoInd = true; opcion = 5; }
			else if (codigo == "I0") { estadoInd = false; }
			else if (codigo == "C1") { estadoCap = true; opcion = 6; }
			else if (codigo == "C0") { estadoCap = false; }
		}
	};
#endif
