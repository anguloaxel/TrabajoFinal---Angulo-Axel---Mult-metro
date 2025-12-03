#ifndef LCDVIEW_H
#define LCDVIEW_H

#include <LiquidCrystal_I2C.h>    // Incluye la librería para controlar displays LCD I2C

class LCDView {        // Declara la clase encargada de la interfaz LCD
	private:
		LiquidCrystal_I2C* lcd;     // Puntero al objeto LCD (inyectado desde afuera)
	public:
		LCDView(LiquidCrystal_I2C* l): lcd(l) {}    // Constructor: asigna el puntero al LCD
		
		void begin(){ lcd->clear(); }      // Limpia la pantalla al iniciar
		
		void showMessage(const char* msg, unsigned long ms=1000){  // Muestra un mensaje temporal
			lcd->clear();   // Borra la pantalla
			lcd->setCursor(0,0);    // Ubica el cursor en la primera fila
			lcd->print(msg);   // Imprime el mensaje
			delay(ms);      // Mantiene el mensaje en pantalla por 'ms' milisegundos
			lcd->clear();
		}
		
		// Renderiza información según la opción seleccionada
		void render(int opcion, float v, float a, float p, float t, float ind, String capStr){
			lcd->setCursor(0,0);       // Ubica el cursor en la primera fila
			lcd->print("Opcion ");     // Escribe "Opcion "
			lcd->print(opcion);        // Imprime el número de opción actual
			lcd->print(" ");           // Espacio para formato
			lcd->setCursor(0,1);       // Baja a la segunda línea
			
			switch(opcion){         // Selecciona qué mostrar según el menú
				case 1: lcd->print("Volt: "); lcd->print(v,2); lcd->print(" V ");
				break;
				case 2: lcd->print("Corr: "); lcd->print(a,3); lcd->print(" A ");
				break;
				case 3: lcd->print("Pote: "); lcd->print(p,3); lcd->print(" W ");
				break;
				case 4: lcd->print("Temp: "); lcd->print(t,1); lcd->print(" C ");
				break;
				case 5: lcd->print("Ind: "); lcd->print(ind,2); lcd->print(" uH ");
				break;
				case 6: lcd->print(capStr); break;      // Imprime directamente el texto de capacitancia
				default: lcd->print("- - -"); break;     // Opción inválida
			}
		}
};
#endif
