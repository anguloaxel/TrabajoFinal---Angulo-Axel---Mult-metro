
#include <Wire.h>                  // necesaria para comunicación I2C (usada por el LCD).
#include <LiquidCrystal_I2C.h>     // Librería para manejar displays LCD I2C basados en PCF8574.
#include <SPI.h>                   // Librería para comunicación SPI, usada por el módulo SD
#include <SD.h>                    // Librería estándar de Arduino para manejo de tarjetas SD.


// Incluye todos los módulos propios del proyecto.
#include "src/Utils.h"         // Funciones auxiliares o utilitarias.
#include "src/SensorBase.h"    // Clase base para sensores.
#include "src/Voltimetro.h"    // Clase para medición de voltaje.
#include "src/Amperimetro.h"   // Clase para medición de corriente.
#include "src/Potencia.h"      // Clase que calcula potencia usando V * I.
#include "src/Termometro.h"    // Clase para leer la temperatura.
#include "src/Inductometro.h"  // Clase para medir inductancia por resonancia.
#include "src/Capacimetro.h"   // Clase para medir capacitancia.
#include "src/LCDView.h"       // Maneja lo que se muestra en el LCD.
#include "src/SDLogger.h"      // Maneja el guardado de datos en la SD.
#include "src/InputManager.h"  // Maneja el botón y comandos por serie.
#include "src/DataSender.h"    // Maneja el envio de datos por puerto serie

// --- Pines usados por el sistema ---
const int botonPin = 2;     // Entrada digital para cambio de modo / selección
const int tempPin  = A3;    // Entrada analógica para el termómetro
const int voltPin  = A6;    // Entrada analógica para voltímetro
const int corrPin  = A7;    // Entrada analógica para amperímetro
const int chipSelect = 10;  // Pin CS para módulo SD
const int pinMedida = 4;    // Entrada para medir frecuencia de resonancia en inductómetro
const int pinPulso  = 3;    // Salida de pulso para carga en inductómetro


// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);


// ----- Crear instancias de las clases -----
Voltimetro volt(voltPin);    // Crea el objeto voltímetro usando el pin asignado.
Amperimetro amp(corrPin);    // Crea el objeto amperímetro usando el pin correspondiente.
Potencia pot(&volt, &amp);   // Crea el objeto potencia, que necesita referencias a voltímetro y amperímetro.
Termometro temp(tempPin);    // Crea el termómetro usando su pin analógico.
Inductometro ind(pinMedida, pinPulso);     // Crea el inductómetro con su pin de entrada y el de salida del pulso.
Capacimetro cap; // Crea el capacímetro (usa los pines configurados internamente).
DataSender sender;


LCDView display(&lcd);         // Crea el módulo de visualización LCD.
SDLogger sdlog(chipSelect);    // Crea el módulo para guardar datos en la tarjeta SD.
InputManager input(botonPin);  // Crea el módulo que gestiona el botón y los comandos serie.


// Variables booleanas que indican si cada medición está activa.
bool estadoVolt=false, estadoAmp=false, estadoPot=false, estadoTemp=false, estadoInd=false, estadoCap=false;

int opcion = 1;  // Variable que indica qué pantalla/medición mostrar en el LCD.


unsigned long lastLoop = 0;  // Variable para contar el tiempo entre guardados en SD.



void setup() {
  Serial.begin(9600);   // Inicializa puerto serie
  lcd.init();          // Inicializa pantalla
  lcd.backlight();    // Encender pantalla


  // Inicializar módulos
  input.begin();      // Inicializa el manejo del botón y comandos por serie.
  display.begin();    // Muestra mensaje inicial durante 1 segundo.
  cap.begin();        // Inicializa el capacímetro (calibración o configuración interna).
  sdlog.begin();      // Inicializa el módulo SD (monta la tarjeta).
  sender.begin(9600);

  // Mensaje inicial
  display.showMessage("Sistema Listo", 1000);  // Muestra mensaje inicial durante 1 segundo.
  Serial.println("Sistema inicializado");      // Imprime en serie que el sistema está listo.
}


void loop() {

  input.update(opcion);    // Actualiza la lectura del botón.

  // Procesa comandos provenientes del botón o del puerto serie.
  // Esto puede cambiar los estados de medición y la opción del display.
  input.checkSerialCommands(estadoVolt, estadoAmp, estadoPot, estadoTemp, estadoInd, estadoCap, opcion);


  // Lecturas según estados
  volt.measure();       // Si el voltímetro está habilitado, mide el voltaje.
  amp.measure();         // Si el amperímetro está habilitado, mide la corriente.
  pot.compute();  // Si volt y amp estan activo, calcula la potencia.
  temp.measure();       // Si está activo el termómetro, lee temperatura.
  ind.measure();         // Si está activo el inductómetro, mide inductancia.
  cap.measure();         // Si está activo el capacímetro, mide capacitancia.


     // --- Mostrar en display según la opción actual ---
    display.render(
        opcion,                  // Pantalla seleccionada
        volt.getValue(),         // Lectura de voltaje
        amp.getValue(),          // Lectura de corriente
        pot.getValue(),          // Cálculo de potencia
        temp.getValue(),         // Lectura de temperatura
        ind.getValue(),          // Lectura de inductancia
        cap.getDisplayString()   // Cadena formateada de capacitancia
    );

  // --- Registro periódico en la tarjeta SD ---
 
  if (millis() - lastLoop > 5000) {     // Comprueba si pasaron más de 5 segundos.
    sdlog.log(volt.getValue(), amp.getValue(), pot.getValue(), temp.getValue(), ind.getValue());
    lastLoop = millis();  // Actualiza el tiempo de última escritura.
  }

  // --- Enviar estado actual al puerto serie en un solo mensaje ---
 sender.send(
        estadoVolt, volt.getValue(),
        estadoAmp,  amp.getValue(),
        estadoPot,  pot.getValue(),
        estadoTemp, temp.getValue(),
        estadoInd,  ind.getValue(),
        estadoCap,  cap.getDisplayString()
    );
  
  delay(200); // Pequeño retardo para evitar saturar lectura y puerto serie.
}