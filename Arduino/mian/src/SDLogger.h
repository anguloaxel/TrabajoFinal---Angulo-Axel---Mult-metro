#ifndef SDLOGGER_H
#define SDLOGGER_H

#include <SD.h>         // Incluye la librería para manejar tarjetas SD

class SDLogger {    // Clase encargada del guardado de datos en la SD
	private:
		int chipSelect;     // Pin CS (Chip Select) de la tarjeta SD
		File myFile;        // Objeto para manipular archivos en la SD
	public:
		SDLogger(int cs): chipSelect(cs) {}   // Constructor: guarda el pin CS
		
		
		void begin(){          // Inicialización de la SD
			if (!SD.begin(chipSelect)) {       // Intenta iniciar la tarjeta SD
				Serial.println("Fallo SD");    // Si falla, muestra mensaje de error
			}
			else { Serial.println("SD ok"); // Si inicia correctamente informa éxito
			}       
		}
			
        // Función que registra valores en el archivo datos.txt
		void log(float v, float a, float p, float t, float ind) {
			
			if (SD.begin(chipSelect)){       // Intenta inicializar SD antes de escribir
				myFile = SD.open("datos.txt", FILE_WRITE);  // Abre/crea archivo para agregar datos
				if (myFile) {      // Verifica si el archivo se abrió correctamente
					myFile.print(millis()); myFile.print(",");  // Escribe el tiempo en ms
					myFile.print(t); myFile.print(",");         // Guarda temperatura
					myFile.print(v); myFile.print(",");         // Guarda voltaje
					myFile.print(a); myFile.print(",");         // Guarda corriente
					myFile.print(p); myFile.print(",");         // Guarda potencia
					myFile.print(ind); myFile.println();        // Guarda inductancia y salto de línea
					
					myFile.close();              // Cierra el archivo para asegurar escritura
				}
			}
		}
};
#endif
