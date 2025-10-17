#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

// --- Display LCD I2C ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Pines ---
const int botonPin = 2;
const int tempPin = A3;
const int voltPin = A6;
const int corrPin = A7;
const int chipSelect = 10;

// --- Variables ---
int opcion = 1;
unsigned long lastDebounce = 0;
const int debounceDelay = 200;

float temperatura = 0;
float voltaje = 0;
float corriente = 0;

// --- SD ---
File myFile;

// --- Sensibilidad del sensor de corriente ---
float Sensibilidad = 0.139;  // para ACS712 20A
float offset = 0.100;        // corrección de ruido

// --- Promedios móviles ---
float leerPromediado(int pin, int muestras) {
  long suma = 0;
  for (int i = 0; i < muestras; i++) {
    suma += analogRead(pin);
    delay(2);
  }
  return (float)suma / muestras;
}

// --- Cambio de escala entre floats ---
float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// --- Corriente pico ---
float get_corriente() {
  float voltajeSensor;
  float corriente = 0;
  long tiempo = millis();
  float Imax = 0;
  float Imin = 0;
  while (millis() - tiempo < 200) {
    voltajeSensor = analogRead(corrPin) * (5.0 / 1023.0);
    corriente = 0.9 * corriente + 0.1 * ((voltajeSensor - 2.527) / Sensibilidad);
    if (corriente > Imax) Imax = corriente;
    if (corriente < Imin) Imin = corriente;
  }
  return (((Imax - Imin) / 2) - offset);
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  pinMode(botonPin, INPUT_PULLUP);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando SD...");
  
  if (!SD.begin(chipSelect)) {
    lcd.setCursor(0, 1);
    lcd.print("Fallo SD!");
    Serial.println("Fallo en inicializacion SD");
    delay(2000);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("SD lista");
    delay(1000);
  }

  lcd.clear();
  lcd.print("Sistema Listo");
  delay(1000);
  lcd.clear();
}

void loop() {
  // --- Lectura de botón para cambiar opción ---
  if (digitalRead(botonPin) == LOW && (millis() - lastDebounce) > debounceDelay) {
    opcion++;
    if (opcion > 10) opcion = 1;
    lastDebounce = millis();
  }

  // --- Medición de sensores ---
  float tempRaw = leerPromediado(tempPin, 10);
  temperatura = (tempRaw * 5.0 / 1023.0) * 100.0;

  float voltRaw = leerPromediado(voltPin, 10);
  voltaje = fmap(voltRaw, 0, 1023, 0.0, 25.0);

  float Ip = get_corriente();
  float Irms = Ip * 0.707;

  corriente = Irms;

  // --- Mostrar en LCD según opción ---
  lcd.setCursor(0, 0);
  lcd.print("Opcion ");
  lcd.print(opcion);
  lcd.print("     ");

  lcd.setCursor(0, 1);
  switch (opcion) {
    case 1:
      lcd.print("Temp: ");
      lcd.print(temperatura, 1);
      lcd.print(" C   ");
      break;
    case 2:
      lcd.print("Volt: ");
      lcd.print(voltaje, 2);
      lcd.print(" V   ");
      break;
    case 3:
      lcd.print("Corr: ");
      lcd.print(corriente, 3);
      lcd.print(" A   ");
      break;
    default:
      lcd.print("Sin asignar   ");
      break;
  }

  // --- Envío por Serial (para Processing) ---
  Serial.print(temperatura);
  Serial.print(",");
  Serial.print(voltaje);
  Serial.print(",");
  Serial.println(corriente);

  // --- Guardar en SD ---
  myFile = SD.open("datos.txt", FILE_WRITE);
  if (myFile) {
    myFile.print(temperatura);
    myFile.print(",");
    myFile.print(voltaje);
    myFile.print(",");
    myFile.println(corriente);
    myFile.close();
  }

  delay(500);
}
