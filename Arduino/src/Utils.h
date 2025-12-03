#ifndef UTILS_H   // Evita que el archivo se incluya más de una vez durante la compilación.
#define UTILS_H   // Si UTILS_H no está definido, lo define y permite incluir el contenido.


#include <Arduino.h>   // Incluye la librería base de Arduino, necesaria para funciones como analogRead(), delay(), tipos básicos, etc.


// Lectura promediada simple
inline float leerPromediado(int pin, int muestras) { // Define una función inline (se expande en tiempo de compilación)
	long suma = 0;   // Variable para acumular la suma de todas las lecturas.
	for (int i = 0; i < muestras; i++) {   // Bucle que se ejecuta "muestras" veces.
		suma += analogRead(pin);     // Lee el valor analógico del pin y lo suma.
		delay(2);      // Pequeño retardo entre lecturas para estabilizar la señal.
	} 
	return (float)suma / muestras;     // Convierte la suma a float y divide por la cantidad de muestras para obtener el promedio real.
}


// Map para floats
// Fórmula matemática del map:
// out = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
inline float fmapf(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


#endif   // Fin del include guard: cierra el #ifndef UTILS_H
