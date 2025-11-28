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
const int pinMedida = 4;   // Entrada para medir resonancia
const int pinPulso  = 3;   // Salida de pulso de carga

// --- Variables ---
int opcion = 1;
unsigned long lastDebounce = 0;
const int debounceDelay = 200;

float temperatura = 0;
float voltaje = 0;
float corriente = 0;
float potencia = 0;

// --- SD ---
File myFile;

// --- Sensibilidad del sensor de corriente ---
float Sensibilidad = 0.100;  // para ACS712 20A sensibilidad en V/A para nuestro sensor
float offset = 0.100;        // corrección de ruido Equivale a la amplitud del ruido (±20A	100 mV/A	0,5V a 4,5V	49mA)

// --- Promedios móviles ---
float leerPromediado(int pin, int muestras) {
  long suma = 0;
  for (int i = 0; i < muestras; i++) {
    suma += analogRead(pin);
    delay(2);
  }
  return (float)suma / muestras;
}

// --- Cambio de escala entre floats --- (Codigo voltimetro)
float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*// --- Corriente pico ---
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
}*/
float get_corriente() {
  float voltage;
  float corriente = 0;
  voltage = analogRead(corrPin) * 5.0 / 1023.0;
  corriente = (voltage - 2.5) / Sensibilidad;
  return(corriente);
}


// --- Inductancias ---
double pulse, frequency, capacitance, inductance;
// Función para medir el tiempo promedio del pulso
double medirPulsoPromedio(int muestras = 5) {
  double suma = 0;
  for (int i = 0; i < muestras; i++) {
    digitalWrite(pinPulso, HIGH);
    delay(5);                  // carga la bobina
    digitalWrite(pinPulso, LOW);
    delayMicroseconds(100);    // tiempo de descarga
    suma += pulseIn(pinMedida, HIGH, 5000); // mide tiempo de pulso
  }
  return suma / muestras;
}

// --- Capacitímetro ---
#include <Capacitor.h>

#define resistencia_H  10035.00F    //R alta: 10K para cargar/descargar el condensador
#define resistencia_L  275.00F      //R baja: 240 para cargar/descargar el condensador

#define CapIN_H       A2     // Medida de capacidades ALTAS + ESR (>50nF) A1
#define CapOUT        A1     // Punto comúm de medida  A2
#define CapIN_L       A0     // Medida capacidad BAJAS (<1uF)  A3
#define cargaPin      9     // Carga lenta (resistencia_H)  12
#define descargaPin   8     // Carga & Descarga & Impulso ESR (resistencia_L)  13

// Definición de los dos pines de conexión (A3, A2), para capacidades BAJAS
Capacitor pFcap(A1,A0);

// Medidas Offset para el calibrado 
float Off_pF_Hr = 0;                   
float Off_pF_H = 0;                   
float Off_pF_Low = 0;                   
float Off_GND = 0; 

unsigned long iniTime;
unsigned long endTime;
float medida; 
float valor;               
String unidad;
String tipo;

// Capacidades BAJAS < 1uF
int Repe = 30;        

// ESR
const int MAX_ADC_VALUE = 1023;
unsigned int sampleESR;
unsigned int milliVolts;
unsigned int ADCref = 0;
float esr;

void calibrado() {
  descargaCap();
  cargaCap_Slow();
  Off_pF_H = ((float)endTime / resistencia_H)*1000000;  
  Serial.print(F("Offset <80uF (pF): ")); 
  Serial.println(Off_pF_H);

  descargaCap();
  cargaCap_Fast();
  Off_pF_Hr = ((float)endTime / resistencia_L)*1000000;  
  Serial.print(F("Offset >80uF (pF): ")); 
  Serial.println(Off_pF_Hr);

  midePF();
  Off_pF_Low = valor; 

  Serial.print(F("Offset <1uF (pF): ")); 
  Serial.println(Off_pF_Low);

  medidaADC();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void descargaCap() {
  analogReference(DEFAULT);           
  pinMode(CapIN_H,INPUT);
 
  pinMode(cargaPin,OUTPUT);              //Pin de carga SALIDA
  digitalWrite(cargaPin, LOW);           //Descarga el condensador 
  pinMode(descargaPin, OUTPUT);          //Pin de descarga SALIDA  
  digitalWrite(descargaPin, LOW);        //Descarga el condensador     
  while(analogRead(CapIN_H) > 0){}       //Espera a que se descargue del todo   
  pinMode(descargaPin, INPUT);           //Pin de descarga en alta impedancia      
  pinMode(cargaPin,INPUT);               //Pin de carga en alta impedancia
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void cargaCap_Slow() {
  pinMode(cargaPin, OUTPUT); 
  digitalWrite(cargaPin, HIGH);  
  iniTime = micros();
  
  //Espera hasta el 63% de 1024
  while(analogRead(CapIN_H) < 645){}    
  
  endTime = micros() - iniTime;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void cargaCap_Fast() {
  pinMode(descargaPin, OUTPUT); 
  digitalWrite(descargaPin, HIGH);  
  iniTime = micros();
  
  //Espera hasta el 63% de 1024
  while(analogRead(CapIN_H) < 645){}    
  
  endTime = micros() - iniTime;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilizamos el ADC para medir la referencia de 1.1V, incorporada en la mayoría de modelos de Arduino
// Luego se calcula la tensión a la que realmente está alimentado Arduino.
// 1.1 x 1023 x 1000 = 1125300
///////////////////////////////////////////////////////////////////////////////////////////////////////
int refADC(){
   long result;
   // Lee 1.1V de referencia interna 
   ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
   delay(2); 
   ADCSRA |= _BV(ADSC);
   while (bit_is_set(ADCSRA,ADSC));
   result = ADCL;
   result |= ADCH<<8;
   result = 1125300L / result; // Calcula la tensión AVcc en mV
   return result;
}
/////////////////////////////////////////////////////////////////////////
void medidaADC() {
  ADCref = refADC();
 // Serial.print(F("ADCref (mV): "));
 // Serial.println(ADCref);
}
/////////////////////////////////////////////////////////////////////////
void mideESR() {
  descargaCap();
  digitalWrite(CapOUT, LOW);
  pinMode(descargaPin, OUTPUT); 
  
  digitalWrite(descargaPin, LOW);
  delayMicroseconds(100);
  digitalWrite(descargaPin, HIGH);
  delayMicroseconds(5);
  sampleESR =  analogRead(CapIN_H); 

  // Mide el Offset del punto de referencia de la medida con respecto a GND 
  Off_GND =  analogRead(CapOUT); 
  //Serial.print(F("Off_GND (Ohm): "));
 // Serial.println(Off_GND);

  // Corrige el Offset (resistencia a masa de A2)
  sampleESR = sampleESR - Off_GND;

 // Serial.print(F("LEE: "));
 // Serial.println(sampleESR);
  descargaCap();

  milliVolts = (sampleESR * (float)ADCref) / MAX_ADC_VALUE;
 // Serial.print(F("mV: "));
  //Serial.println(milliVolts);

  // Calcula la resistencia de A2 a GND y la suma a la resistencia de carga
  int R_GND = resistencia_L / MAX_ADC_VALUE * Off_GND;
  //Serial.print(F("R_GND (Ohm): "));
 // Serial.println(R_GND);
  
  esr = (resistencia_L + R_GND) / (((float)ADCref / milliVolts) - 1); 
  
  // Calibrado ESR (Ohmios)
  esr = esr - 0.9;
  if (esr < 0) {esr = 0;}
}
///////////////////////////////////////////////////////
void midePF(){
  descargaCap();
  float valorMedio = 0;                   // Reinicia valor medio
  for(int i = 0; i < Repe; i++){
    valor = pFcap.Measure();
    valorMedio = valorMedio + valor;
    descargaCap();
  }
  valor = valorMedio / Repe;              // Guarda el valor promedio de las muestras
}
/////////////////////////////////////////////////////////////////////////


bool estadoVolt=0;
bool estadoAmp=0;
bool estadoPot=0;
bool estadoTemp=0;
bool estadoInd=0;
bool estadoCap=0;

void medirVolt(){
  float voltRaw = leerPromediado(voltPin, 10);
  voltaje = fmap(voltRaw, 0, 1023, 0.0, 25.0);// cambiar escala a 0.0 - 25.0
}

void medirAmp (){
  float Ip = get_corriente(); //obtenemos la corriente pico
  // float Irms = Ip * 0.707; //Intensidad RMS = Ipico/(2^1/2)  

  // corriente = Irms;
  corriente = Ip;
}

void medirPot(){  
  //potencia
  potencia=voltaje*corriente;
}

void medirTemp(){
  float tempRaw = leerPromediado(tempPin, 20);
  temperatura = (tempRaw * 5.0 / 1023.0) * 100.0;
}

void medirInd(){
  
      pulse = medirPulsoPromedio();   // lee tiempo promedio

      if (pulse > 0.1) { // si se detecta una señal válida

        capacitance = 1.E-7; //(1E-7 capacitor 104)(1E-6 capacitor 105)
        frequency = 1.E6 / (2.0 * pulse);
        inductance = 1. / (capacitance * frequency * frequency * 4.0 * 3.14159 * 3.14159);
        inductance *= 1E6; // convierte a microhenrios (µH)
        inductance = inductance*0.61664; //100uH=0.68542  680uH=0.54786;
          

      lcd.print("Ind: ");
      lcd.print(inductance, 3);
      lcd.setCursor(13,1); 
      lcd.print(" uH   ");
      delay(10);
      }
}



void medirCap(){
  // Se hace un test del condensador
  descargaCap();
  pinMode(descargaPin, OUTPUT); 
  digitalWrite(descargaPin, HIGH);  
  delay(1);
  unsigned int muestra1 = analogRead(CapIN_H);
  delay(100);
  unsigned int muestra2 = analogRead(CapIN_H);
  unsigned int cambio = muestra2 - muestra1;
  // TEST ??
  if (muestra2 < 1000 && cambio < 30 ) {
    tipo = "[Test]";
    /*Serial.print(F("Muestra 1: ")); 
    Serial.println(muestra1); 
    Serial.print(F("Muestra 2: ")); 
    Serial.println(muestra2); */
    // <<< Test OK
  }else {
    // Se descarga/carga el condensador bajo prueba, con la resistencia de 240 Ohmios
    descargaCap();
    cargaCap_Fast();
    medida = ((float)endTime / resistencia_L) - (Off_pF_Hr / 1000000);
    
    // Si la capacidad es inferior a 80 uF, se repite otra vez la medida cargando con la resistencia de 10K
    if (medida < 80){
      tipo = " <80uF";
      descargaCap();
      cargaCap_Slow();
      medida = ((float)endTime / resistencia_H) - (Off_pF_H / 1000000);
    }else {
      tipo = " >80uF";
    }
  
    if (medida > 1){
      valor = medida;
      unidad = " uF";
    }else if (medida > 0.05){ 
      tipo = " >50nF";
      valor = medida * 1000; 
      unidad = " nF";
    }else {
      tipo = "  <1uF";
      midePF();
      valor = valor - Off_pF_Low;
     
      // No muestra valores inferiores a 1 pF 
      if (valor < 1) { 
        valor = 0;
        unidad = " pF";
      }else if (valor > 1000000) {
        valor = -999;
        unidad = "> 1uF";
      }else if (valor > 1000) {
      valor = valor / 1000;
        unidad = " nF";
      }else{
        // Ajuste fino (pF)
        if (valor < 35){
          valor = valor - 9;
        }else if (valor < 50){
          valor = valor - 13;
        }else if (valor < 65){
          valor = valor - 17;
        }else {
          valor = valor - 22;
        }
        unidad = " pF";
      }
    }
  }
  // <<< Fin del Test y Medida >>> PRESENTA DATOS
  // Mide ESR a partir de 10uF  
  if (medida > 10 || tipo == "[Test]"){
    mideESR();
    /* if(esr < 300){
      Serial.print(esr,1);
    }else {
      Serial.println(F(">300"));
    }
    Serial.println(F(" Ohm"));*/
  }
     
  // Se comprueba que haya pasado el TEST
  if (tipo == "[Test]"){
  // Serial.println(F("#ERROR capacidad")); 
  }else {
  // Test OK >>> se muestra la medida en el display
    if (valor < 0 && valor != -999) {valor = 0;}
      
    /*// Presenta los resultados de la medida y muestra la actividad en el display
    Serial.print(F("Capacidad"));
    Serial.print(tipo);
    Serial.print(F(" = "));
    if (valor != -999) {
      if (valor < 1000 &&  unidad != " pF" ) {
        Serial.print(valor,2);  
      }else {
        Serial.print(valor,0);  
      }
    }
    Serial.println(unidad); */ 
  }
        
  //Mantiene la marca de actividad en el LCD durante 300ms
  delay(500);
}

//Lectura puerto serie
String codigo = "";  // Variable para guardar el código recibido porpuerto serie
void leerCodigo(){
   codigo = ""; // Limpia para el siguiente mensaje
 while (Serial.available() > 0) {
    char c = Serial.read();     // Lee un carácter
    if (c != '\n' && c != '\r') { // Cuando llega Enter, envía el texto de vuelta
      codigo += c; // Acumula el texto
    }
  }
}

void estados(bool V,bool A, bool P,bool T,bool I,bool C){    
//   estados(estadoVolt,estadoAmp,estadoPot,estadoTemp,estadoInd,estadoCap);
    estadoVolt=V;
    estadoAmp=A;
    estadoPot=P;
    estadoTemp=T;
    estadoInd=I;  
    estadoCap=C;
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  pinMode(botonPin, INPUT_PULLUP); //boton

  pinMode(pinMedida, INPUT);
  pinMode(pinPulso, OUTPUT);

  // --- Capacitímetro ---

  pFcap.Calibrate(41.95,36.00);  
  
  pinMode(CapOUT, OUTPUT);
  pinMode(CapIN_L, OUTPUT);
  calibrado();



  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando SD...");
  
  if (!SD.begin(chipSelect)) {
    lcd.setCursor(0, 1);
    lcd.print("Fallo SD!");
    Serial.println("Fallo en inicializacion SD");
    delay(1000);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("SD lista");
    delay(500);
  }

  lcd.clear();
  lcd.print("Sistema Listo");
  delay(1000);
  lcd.clear();
}





void loop() {

  leerCodigo();

  if(codigo=="V1"){
    estados(1,estadoAmp,estadoPot,estadoTemp,0,0);
    opcion=1;
  }
  if(codigo=="V0"){
    estados(0,estadoAmp,estadoPot,estadoTemp,estadoInd,estadoCap);
  }
  if(codigo=="A1"){
    estados(estadoVolt,1,estadoPot,estadoTemp,0,0);
    opcion=2;
  }
  if(codigo=="A0"){
    estados(estadoVolt,0,estadoPot,estadoTemp,estadoInd,estadoCap);
  }
  if(estadoVolt==1  &&  estadoAmp==1){
    estados(estadoVolt,estadoAmp,1,estadoTemp,estadoInd,estadoCap);
  }else{
    estados(estadoVolt,estadoAmp,0,estadoTemp,estadoInd,estadoCap);
  }
  if(codigo=="T1"){
    estados(estadoVolt,estadoAmp,estadoPot,1,0,0);
    opcion=4;
  }
  if(codigo=="T0"){
    estados(estadoVolt,estadoAmp,estadoPot,0,estadoInd,estadoCap);
  }
  if(codigo=="I1"){
    estados(0,0,0,0,1,0);
    opcion=5;
  }
  if(codigo=="I0"){
    estados(estadoVolt,estadoAmp,estadoPot,estadoTemp,0,estadoCap);
  }
  if(codigo=="C1"){
    estados(0,0,0,0,0,1);
    opcion=6;
  }
  if(codigo=="C0"){
    estados(estadoVolt,estadoAmp,estadoPot,estadoTemp,estadoInd,0);
  }


  // --- Lectura de botón para cambiar opción ---
  if (digitalRead(botonPin) == LOW && (millis() - lastDebounce) > debounceDelay) {
    opcion++;
    if (opcion > 6) opcion = 1;
    lastDebounce = millis();
  }

  // --- Medición de sensores ---
  if(estadoVolt == 1){
    medirVolt();
    Serial.print("V");
    Serial.print(voltaje);
    Serial.print(",");
  }

  if (estadoAmp == 1){
    medirAmp();
    Serial.print("A");
    Serial.print(corriente);
    Serial.print(",");
  }

  if (estadoPot == 1){
    medirPot();
    Serial.print("P");
    Serial.print(potencia);
    Serial.print(",");
  }

  if(estadoTemp == 1){
    medirTemp();
    Serial.print("T");
    Serial.println(temperatura);
    Serial.print(",");
  }

  if(estadoInd==1){
    medirInd();
    Serial.print("I");
    Serial.println(inductance);
    Serial.print(",");
  }

  if(estadoCap==1){
    medirCap();
    if (valor != -999) {
      if (valor < 1000 &&  unidad != " pF" ) {
        Serial.print(valor,2);  
      }else {
        Serial.print(valor,0);  
      }
    }
    Serial.println(unidad);
    if(esr < 300){
      Serial.println(F("ESR:"));
      Serial.print(esr,1);
      Serial.println(F(" Ohm"));
    }

  }


  // --- Mostrar en LCD según opción ---
  lcd.setCursor(0, 0);
  lcd.print("Opcion ");
  lcd.print(opcion);
  lcd.print("     ");

  lcd.setCursor(0, 1);
  switch (opcion) {

    case 1:
      estados(1,estadoAmp,estadoPot,estadoTemp,0,0);
      lcd.print("Volt: ");
      lcd.print(voltaje, 2);
      lcd.print(" V   ");
      break;
    case 2:
      estados(estadoVolt,1,estadoPot,estadoTemp,0,0);
      lcd.print("Corr: ");
      lcd.print(corriente, 3);
      lcd.print(" A   ");
      break;
    case 3:
      lcd.print("Pote: ");
      lcd.print(potencia, 3);
      lcd.print(" W   ");
      break;
    case 4:     
      estados(estadoVolt,estadoAmp,estadoPot,1,0,0);
      lcd.print("Temp: ");
      lcd.print(temperatura, 1);
      lcd.print(" C   ");
      break;
    case 5:
      estados(0,0,0,0,1,0);
      break;
    case 6:
      estados(0,0,0,0,0,1);
      break;
   }

  /*
  // --- Guardar en SD ---
  myFile = SD.open("datos.txt", FILE_WRITE);
  if (myFile) {
    myFile.print(temperatura);
    myFile.print(",");
    myFile.print(voltaje);
    myFile.print(",");
    myFile.print(corriente);
    myFile.print(",");
    myFile.print(potencia);
    myFile.print(",");
    myFile.println(inductance);
    myFile.close();
  }*/

  delay(300);
}
