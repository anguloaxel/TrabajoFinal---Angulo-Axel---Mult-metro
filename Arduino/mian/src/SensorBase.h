#ifndef SENSORBASE_H    // Evita que este archivo se incluya más de una vez durante la compilación.
#define SENSORBASE_H    // Si SENSORBASE_H no está definido, lo define.


#include <Arduino.h>     // Incluye la librería base de Arduino necesaria para tipos, funciones básicas y compatibilidad con el entorno de Arduino.

// Declaración de la clase base abstracta para todos los sensores del proyecto.
// Sirve como interfaz común que garantiza que todos los sensores implementen las mismas funciones mínimas.
class SensorBase {
public:
	virtual void begin() {}    // Método opcional de inicialización. La clase base lo deja vacío para que las clases hijas lo sobreescriban si lo necesitan.
	virtual void measure() = 0; // Método de medición obligatorio. "= 0" lo convierte en un método virtual puro, lo que significa que toda clase que herede de SensorBase debe implementar este método.
	virtual float getValue() { return 0; }  // Devuelve el valor medido por el sensor.
	// La clase base devuelve 0 por defecto, pero las clases derivadas deben sobreescribirlo para retornar su valor real.
};


#endif
