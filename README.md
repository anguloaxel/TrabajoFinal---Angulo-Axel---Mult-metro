# Proyecto: Multimetro

## Integrante:
- Angulo Axel Damian

PROYECTO: Multímetro Digital con Arduino + Processing

Este proyecto implementa un multímetro digital utilizando un Arduino como módulo de medición y una interfaz gráfica creada en Processing para visualizar los valores en tiempo real.

FUNCIONES PRINCIPALES

Medición de voltaje (V)
Medición de corriente (A)
Medición de potencia (W)
Medición de temperatura (°C)
Medición de inductancia (uH)
Medición de capacitancia (cadena de texto, ej.: 33uF, 120nF)


CARACTERÍSTICAS DEL SISTEMA

Comunicación serial bidireccional Arduino ↔ Processing
Gráficos en tiempo real para voltaje, corriente, potencia y temperatura
Interfaz gráfica con botones de activación para cada función
Consola interna que muestra todo lo enviado/recibido por el puerto serie
Guardado de mediciones en archivo .txt con fecha y hora
Visualización de valores grandes para inductancia y capacitancia
Lectura modular desde Arduino usando clases y sensores independientes


FORMATO DE DATOS RECIBIDOS DESDE ARDUINO

El Arduino envía cada lectura en una línea con el siguiente formato:

Vxx.xx,Axx.xx,Pxx.xx,Txx.xx,Ixx.xx,Ccadena

Ejemplo:
V2.54,A0.10,P0.26,T24.8,I12.5,C33uF


REQUISITOS

ARDUINO:

Arduino Nano o Uno
Sensores según la función utilizada
Botón físico con debounce no bloqueante


PROCESSING:

Processing 4.x
Librería “processing.serial” (incluida por defecto)


USO GENERAL

Cargar el código Arduino en la placa.
Ejecutar el programa en Processing.
Seleccionar el puerto serie correcto.
Activar/desactivar funciones desde los botones de la GUI.
Opcional: habilitar el guardado para registrar las mediciones en un archivo.

ESTRUCTURA DEL PROYECTO

Código Arduino → Manejo de sensores, botón y envío serial.
Código Processing → Interfaz gráfica, gráficos, consola y registro.

