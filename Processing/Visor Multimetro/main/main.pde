import processing.serial.*;  // Librería para comunicación serie con Arduino (y otras placas)
import java.io.PrintWriter;  // Librería para escribir archivos de texto

// --- Puerto serie ---
Serial myPort;  // Objeto Serial que representa el puerto serie usado por Processing

// --- Configuración de interfaz ---
int numBotones = 6;   // Cantidad de botones funcionales en la UI
int btnAncho = 100;   // Dimensiones de cada botón (ancho y alto)
int btnAlto = 50;     // Espacio vertical entre botones
int espaciado = 20;

// --- Estados de botones (toggle) ---   // Array con el estado ON/OFF de cada botón (true = activo/encendido)
boolean[] activo = { false, false, false, false, false, false };

// --- Variables de mediciones ---    // Variables numéricas donde se guardan las lecturas recibidas por serie
float voltaje = 0;       // Voltaje en V
float amperaje = 0;      // Corriente en A
float potencia = 0;      // Potencia en W
float temperatura = 0;   // Temperatura en °C
float inductancia = 0;   // Inductancia en uH

// Capacitancia ahora se guarda como String 
String capacitancia = "";   // Variable global donde se guarda el resultado


// --- Historial para gráficos ---
float[] histVolt, histAmp, histPot, histTemp;  // Arrays circulares que almacenan las últimas mediciones para dibujar gráficos
int numMuestras = 200;   // Número de muestras que guardamos en cada historial

// --- Guardado en archivo ---
boolean guardando = false;    // Flag para indicar si se está grabando en disco
PrintWriter output;      // Objeto PrintWriter para escribir el archivo
String fileName = "";    // Nombre temporal del archivo y nombre mostrado
String nombreArchivoActual = "";

// --- Consola serie ---
ArrayList<String> consola = new ArrayList<String>();  // Buffer en memoria para almacenar las líneas que mostramos en la consola dentro de la UI
int consolaMaxLineas = 200;   // Máximo de líneas que guardaremos en memoria para la consola

void setup() {
  size(1350, 670);      // Tamaño de la ventana de la aplicación (ancho x alto)

  // --- Puerto serie ---
  printArray(Serial.list());      // Muestra en consola la lista de puertos disponibles (útil para depurar)
  myPort = new Serial(this, Serial.list()[2], 9600);      // Abrimos el puerto serie: elegimos el índice 2 de la lista (ajustar si tu puerto está en otro índice)
  myPort.bufferUntil('\n');       // Buffer hasta nueva línea: serialEvent() se disparará cuando reciba '\n'

  // --- Inicializar historiales ---
  // Reservamos memoria para los historiales con la cantidad de muestras definida
  histVolt = new float[numMuestras];
  histAmp  = new float[numMuestras];
  histPot  = new float[numMuestras];
  histTemp = new float[numMuestras];
}

// --------------------------------------------------------------------
// DIBUJO PRINCIPAL
// --------------------------------------------------------------------
void draw() {
  background(220);     // Color de fondo de toda la ventana
 
   // --- Título principal ---
  textSize(24);
  fill(0);
  textAlign(CENTER, CENTER);
  text("Multímetro para Electrónica", width/2, 20);
  
  // Volvemos a un tamaño estándar de letra
  textSize(16);

  // --- Fecha y hora ---
  fill(0);
  textAlign(RIGHT, TOP);
  String fecha = nf(day(),2) + "/" + nf(month(),2) + "/" + year();
  String hora  = nf(hour(),2) + ":" + nf(minute(),2) + ":" + nf(second(),2);
  text(fecha + "   Hora: " + hora, width - 20, 50);

  // --- Botones de funciones ---
  for (int i = 0; i < numBotones; i++) {
    
    // Posición X fija y Y escalonada según el índice
    int x = 20;
    int y = 100 + i * (btnAlto + espaciado);

    // Si el botón está activo (seleccionado) lo pintamos verde,
    // si no está activo lo pintamos gris.
    if (activo[i]) 
      fill(0, 220, 0);
    else
      fill(180);
            
    rect(x, y, btnAncho, btnAlto, 10);    // Dibujar el rectángulo del botón con esquinas redondeadas.

    fill(0);    // Color del texto dentro del botón
   
    String etiqueta = "";    // Según el número de botón, asignamos su etiqueta
    switch (i+1) {
      case 1: etiqueta="Voltaje"; break;
      case 2: etiqueta="Amperaje"; break;
      case 3: etiqueta="Potencia"; break;
      case 4: etiqueta="Temperatura"; break;
      case 5: etiqueta="Inductancia"; break;
      case 6: etiqueta="Capacitancia"; break;
    }
    
    textAlign(CENTER, CENTER);    // Centramos el texto dentro del botón
    text(etiqueta, x + btnAncho/2, y + btnAlto/2);       // Dibujar la etiqueta dentro del botón
  }

  // --- Cuadros de valores ---   (muestra los valores grandes de cada medición)
  mostrarValores();

  // --------------------------------------------------------------------
  // GRÁFICOS DE HISTORIA PARA LAS VARIABLES QUE LO USAN
  // Cada gráfico solo se dibuja si el botón correspondiente está activo
  // --------------------------------------------------------------------
  int xg = 300;   // Posición horizontal base para los gráficos
  
  if (activo[0]) graficarVariable(xg, 100, 400, 120, histVolt, 0, 50, voltaje, "Voltaje (V)", color(255, 100, 0));          // Gráfico de voltaje
  if (activo[1]) graficarVariable(xg, 240, 400, 120, histAmp, 0, 20, amperaje, "Amperaje (A)", color(0, 150, 255));         // Gráfico de amperaje
  if (activo[2]) graficarVariable(xg, 380, 400, 120, histPot, 0, 250, potencia, "Potencia (W)", color(255, 0, 150));        // Gráfico de potencia
  if (activo[3]) graficarVariable(xg, 520, 400, 120, histTemp, 0, 50, temperatura, "Temperatura (°C)", color(255, 0, 0));   // Gráfico de temperatura

  if (activo[4]) mostrarValorGrande(xg + 450, 200, "Inductancia", inductancia, "uH");       // Valor grande de inductancia (sin gráfico)
  if (activo[5]) mostrarValorGrande(xg + 450, 400, "Capacitancia", capacitancia);           // Valor grande de capacitancia (String)
 
  // --------------------------------------------------------------------
  // BOTÓN "GUARDAR" PARA ARCHIVAR DATOS DEL MULTÍMETRO
  // --------------------------------------------------------------------
  int xgbtn = width - 200;    // posición X del botón de guardar
  int ygbtn = height - 100;   // posición Y

  // Si está guardando se pone verde, si no, rojo
  fill( guardando ? color(0,200,0) : color(200,0,0) );
  rect(xgbtn, ygbtn, 160, 50, 12);

  // Texto del botón ("Guardar" o "Grabando...")
  fill(255);
  textAlign(CENTER, CENTER);
  textSize(18);
  text( guardando ? "Grabando..." : "Guardar" , xgbtn + 80, ygbtn + 25);

  // Mientras guarda, muestra el nombre del archivo generado
  if (guardando) {
    fill(0);
    textAlign(CENTER, TOP);
    textSize(14);
    text("Archivo: " + nombreArchivoActual, xgbtn + 80, ygbtn + 60);
  }

  // ----------------------------------------------------------
  // --- DIBUJAR CONSOLA SERIE 
  // Dibuja la consola pequeña al final de la pantalla, donde se
  // muestra todo lo transmitido (TX) y recibido (RX) por el puerto serie.
  dibujarConsola(710, 560, 400, 100);
}

// --------------------------------------------------------------------
// MOSTRAR VALORES PEQUEÑOS JUNTO A CADA BOTÓN
// --------------------------------------------------------------------
void mostrarValores() {
  for (int i = 0; i < numBotones; i++) {
   
    // Posición del cuadro donde se muestran los valores
    int x = 135;
    int y = 100 + i * (btnAlto + espaciado);

    // Fondo del cuadro de valor
    fill(240);
    rect(x, y, btnAncho, btnAlto, 10);

    fill(0);    // Color del texto
    
    // Texto a mostrar según la variable activa
    String texto = "";
    switch (i+1) {
      case 1: texto = nf(voltaje, 1, 2) + " V"; break;
      case 2: texto = nf(amperaje, 1, 2) + " A"; break;
      case 3: texto = nf(potencia, 1, 1) + " W"; break;
      case 4: texto = nf(temperatura, 1, 1) + " °C"; break;
      case 5: texto = nf(inductancia, 1, 2) + " uH"; break;
      case 6: texto = capacitancia; break;         // Capacitancia ya viene como String (ejemplo "25.54 uF")
    }
    
    textAlign(CENTER, CENTER);      // Centrar el texto en el cuadro
    text(texto, x + btnAncho/2, y + btnAlto/2);
  }
}

// --------------------------------------------------------------------
// EVENTO CLICK DEL MOUSE (para botones de función y botón Guardar)
// --------------------------------------------------------------------
void mousePressed() {

  // --- DETECCIÓN DE CLIC EN BOTONES DE FUNCIÓN
  for (int i = 0; i < numBotones; i++) {
    int x = 20;
    int y = 100 + i * (btnAlto + espaciado);

    // Verifica si el clic cayó dentro del botón
    if (mouseX > x && mouseX < x + btnAncho && mouseY > y && mouseY < y + btnAlto) {
      activo[i] = !activo[i];       // Cambia el estado ON/OFF del botón

      String codigo = "";        // Código a enviar por serial al Arduino

      if (i == 0) codigo = activo[i] ? "V1" : "V0";
      if (i == 1) codigo = activo[i] ? "A1" : "A0";
      if (i == 2) codigo = activo[i] ? "P1" : "P0";
      if (i == 3) codigo = activo[i] ? "T1" : "T0";
      if (i == 4) codigo = activo[i] ? "I1" : "I0";
      if (i == 5) codigo = activo[i] ? "C1" : "C0";

      myPort.write(codigo);          // Enviar por el puerto serie
      println("Enviado: " + codigo); // Mostrar en consola interna y consola de Processing
      logConsola("TX: " + codigo);
    }
  }

  // ----------------------------------------------------------
  // --- BOTÓN "GUARDAR" PARA INICIAR / DETENER GRABACIÓN
  int xgbtn = width - 200;
  int ygbtn = height - 100;
  if (mouseX > xgbtn && mouseX < xgbtn + 160 &&
      mouseY > ygbtn && mouseY < ygbtn + 50) {

    guardando = !guardando;  // ON/OFF

    if (guardando) {   
      
      // Crear nombre único usando fecha y hora
      String fecha = nf(day(),2) + "-" + nf(month(),2) + "-" + year();
      String hora  = nf(hour(),2) + "-" + nf(minute(),2) + "-" + nf(second(),2);
      fileName = "datos_" + fecha + "_" + hora + ".txt";

      output = createWriter(fileName);    // Abrir archivo para escritura
      nombreArchivoActual = fileName;

      println("Guardando en archivo: " + fileName);
      logConsola(">> Grabando en archivo " + fileName);
    } else {     // Si estaba grabando, cerramos el archivo correctamente
      if (output != null) {
        output.flush();
        output.close();
      }
      println("Guardado detenido.");
      logConsola(">> Grabación detenida");
    }
  }
}

// --------------------------------------------------------------------
// EVENTO SERIAL: SE EJECUTA CADA VEZ QUE LLEGA UNA LÍNEA
// --------------------------------------------------------------------
void serialEvent(Serial p) {
  String lectura = p.readStringUntil('\n');    // Leer línea completa hasta salto de línea
  if (lectura != null) {
    lectura = lectura.trim();   // limpiar espacios y saltos
    println("Recibido: " + lectura);
    logConsola("RX: " + lectura);

    try {    // Decodificar cada variable solo si aparece su letra
      if (lectura.indexOf('V') != -1) voltaje = extraerValor(lectura, 'V');
      if (lectura.indexOf('A') != -1) amperaje = extraerValor(lectura, 'A');
      if (lectura.indexOf('P') != -1) potencia = extraerValor(lectura, 'P');
      if (lectura.indexOf('T') != -1) temperatura = extraerValor(lectura, 'T');
      if (lectura.indexOf('I') != -1) inductancia = extraerValor(lectura, 'I');
            // Capacitancia es un String → usa función especial
      if (lectura.indexOf('C') != -1) capacitancia = extraerCadenaCapacitancia(lectura);

      // Actualizar información histórica para gráficos
      actualizarHistorial(histVolt, voltaje);
      actualizarHistorial(histAmp, amperaje);
      actualizarHistorial(histPot, potencia);
      actualizarHistorial(histTemp, temperatura);

      if (guardando && output != null) {         // Si se está guardando, escribir línea en archivo
        String fecha = nf(day(),2)+"/"+nf(month(),2)+"/"+year();
        String hora  = nf(hour(),2)+":"+nf(minute(),2)+":"+nf(second(),2);

        output.println(
          fecha + "," + hora + "," +
          voltaje + "," + amperaje + "," + potencia + "," +
          temperatura + "," + inductancia + "," + capacitancia
        );
      }

    } catch (Exception e) {
      println("Error procesando: " + e);
      logConsola("ERROR: " + e);
    }
  }
}

// --------------------------------------------------------------------
// EXTRAER VALORES FLOAT DESDE EL STRING SERIAL
// Ejemplo: "V12.45,"  →  12.45
// --------------------------------------------------------------------
float extraerValor(String s, char clave) {

  int start = s.indexOf(clave);     // Buscar la letra del parámetro
  if (start == -1) return 0;

  int end = s.indexOf(",", start);  // Buscar coma después del número

  String num = (end != -1) ?  // Si no hay coma → tomar desde clave+1 hasta el final
       s.substring(start + 1, end) :
       s.substring(start + 1);

  num = trim(num);  // Quitar espacios y caracteres raros
  
  return float(num);  // Convertir a float
}

// --------------------------------------------------------------------
// EXTRAER LA CADENA COMPLETA DE CAPACITANCIA DESDE LA LÍNEA SERIAL
// Ejemplo:  "V12.4,A0.52,C22.35uF,T31.2"  →  "22.35uF"
// --------------------------------------------------------------------
String extraerCadenaCapacitancia(String s) {
  int start = s.indexOf('C');     // Buscar la letra 'C' que indica inicio de capacitancia
  if (start == -1) return "";  // Si no existe, devolver vacío

  int end = s.indexOf(',', start);     // Buscar la coma que termina la sección de capacitancia
  if (end == -1) {     // Si no hay coma después de C → tomar todo lo que sigue
    return s.substring(start + 1).trim();
  }

  return s.substring(start + 1, end).trim();   // Extraer desde después de C hasta antes de la coma
}



// --------------------------------------------------------------------
// DESPLAZA TODOS LOS DATOS A LA IZQUIERDA Y AGREGA EL NUEVO VALOR
// Mantiene un historial deslizante para graficar la variable
// --------------------------------------------------------------------
void actualizarHistorial(float[] hist, float nuevo) {      // Desplazar todos los elementos 1 posición hacia la izquierda
  for (int i = 0; i < hist.length - 1; i++) {
    hist[i] = hist[i+1];  
  }
  hist[hist.length - 1] = nuevo;     // Insertar el nuevo valor al final del array
}

// --------------------------------------------------------------------
// GRAFICA UNA VARIABLE EN TIEMPO REAL USANDO HISTORIAL DE DATOS
// - x0, y0 → posición del gráfico
// - datos[] → historial deslizante
// - minVal / maxVal → escala vertical
// - actual → valor actual (línea azul)
// --------------------------------------------------------------------
void graficarVariable(int x0, int y0, int ancho, int alto,
                      float[] datos, float minVal, float maxVal,
                      float actual, String titulo, color c) {

  fill(255);     // Fondo del gráfico
  stroke(0);
  rect(x0, y0, ancho, alto);

  fill(0);     // Título
  textAlign(LEFT, TOP);
  text(titulo, x0 + 10, y0 + 5);

  stroke(180);      // Líneas horizontales de referencia
  
  for (int i = 0; i <= 4; i++) {
    float v = map(i, 0, 4, minVal, maxVal);       // Valor correspondiente a la línea
    float y = map(v, minVal, maxVal, y0 + alto - 20, y0 + 20);       // Posición vertical mapeada
    line(x0 + 50, y, x0 + ancho - 10, y);     // Línea de referencia
    
    // Etiqueta del valor
    fill(0);  
    textAlign(RIGHT, CENTER);
    text(nf(v, 1, 1), x0 + 45, y);
  }

    // --- Curva del historial ---
  stroke(c);
  noFill();
  beginShape();
  
  for (int i = 0; i < datos.length; i++) {
    float x = map(i, 0, datos.length-1, x0 + 55, x0 + ancho - 10);
    float y = map(datos[i], minVal, maxVal, y0 + alto - 20, y0 + 20);
    vertex(x, y);
  }
  endShape();

  // --- Línea del valor actual ---
  stroke(0, 0, 255);
  float yAct = map(actual, minVal, maxVal, y0 + alto - 20, y0 + 20);
  line(x0 + 50, yAct, x0 + ancho - 10, yAct);
  
  // Texto del valor actual
  fill(0, 0, 255);
  textAlign(LEFT, CENTER);
  text("Actual: " + nf(actual, 1, 2), x0 + 60, yAct - 15);
}

// --------------------------------------------------------------------
// MUESTRA UN CUADRO GRANDE DE VALOR (para inductancia, voltaje, etc.)
// --------------------------------------------------------------------
void mostrarValorGrande(int x, int y, String titulo, float valor, String unidad) {
 
  // Marco y fondo
  fill(255);
  stroke(0);
  rect(x, y, 350, 150, 10);

  // Título
  fill(0);
  textAlign(CENTER, TOP);
  textSize(20);
  text(titulo, x + 175, y + 10);

  // Valor grande
  textSize(32);
  text(nf(valor, 1, 3) + " " + unidad, x + 175, y + 70);

  textSize(16);    // Restablecer tamaño
}

// --------------------------------------------------------------------
// VERSIÓN PARA CAPACITANCIA ya viene como String (ej: "22.34 uF")
// --------------------------------------------------------------------
void mostrarValorGrande(int x, int y, String titulo, String textoValor) {
  fill(255);
  stroke(0);
  rect(x, y, 350, 150, 10);

  fill(0);
  textAlign(CENTER, TOP);
  textSize(20);
  text(titulo, x + 175, y + 10);

  textSize(32);
  text(textoValor + " " , x + 175, y + 70);

  textSize(16);
}


// --------------------------------------------------------------------
// AGREGA UNA LÍNEA A LA CONSOLA INTERNA (TX / RX)
// Mantiene máximo 'consolaMaxLineas' líneas
// --------------------------------------------------------------------
void logConsola(String msg) {
 
  // Si está llena, borrar la primera línea
  if (consola.size() >= consolaMaxLineas) {
    consola.remove(0);
  }
  consola.add(msg);    // Agregar nueva línea al final
}


// --------------------------------------------------------------------
// DIBUJA EL CUADRO DE CONSOLA SERIE EN PANTALLA
// Muestra el historial de TX y RX desplazándose automáticamente
// --------------------------------------------------------------------
void dibujarConsola(int x, int y, int w, int h) {
  
  // Fondo negro 
  fill(20);
  stroke(255);
  rect(x, y, w, h);

  // Texto verde 
  fill(0, 255, 0);
  textAlign(LEFT, TOP);
  textSize(12);

  // Cuántas líneas entren según el alto del cuadro
  int lineHeight = 14;
  int maxLineasVisibles = h / lineHeight;

  int start = max(0, consola.size() - maxLineasVisibles);     // Calcular desde dónde empezar a mostrar para ver las últimas líneas

  // Dibujar cada línea visible
  for (int i = start; i < consola.size(); i++) {
    text(consola.get(i), x + 5, y + 5 + (i - start) * lineHeight);
  }
}
