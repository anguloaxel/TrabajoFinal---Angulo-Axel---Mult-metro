import processing.serial.*;

Serial myPort;

// --- Configuración de interfaz ---
int numBotones = 6;
int btnAncho = 100;
int btnAlto = 50;
int espaciado = 20;

// --- Variables recibidas ---
String datoRecibido = "";
float voltaje = 0;
float amperaje = 0;
float potencia = 0;
float temperatura = 0;

// --- Históricos ---
int numMuestras = 200;
float[] histVolt, histAmp, histPot, histTemp;

void setup() {
  size(1200, 700);
  println(Serial.list());
  
  // Ajustar el índice según puerto
  myPort = new Serial(this, Serial.list()[2], 9600);
  myPort.bufferUntil('\n');
  
  histVolt = new float[numMuestras];
  histAmp = new float[numMuestras];
  histPot = new float[numMuestras];
  histTemp = new float[numMuestras];
}

void draw() {
  background(220);
  textSize(24);
  fill(0);
  textAlign(CENTER, CENTER);
  text("Multímetro para Electrónica", width/2, 20);
  textSize(16);

  // --- Botones ---
  for (int i = 0; i < numBotones; i++) {
    int x = 20;
    int y = 100 + i * (btnAlto + espaciado);
    fill(180);
    rect(x, y, btnAncho, btnAlto, 10);
    fill(0);
    String etiqueta = "";
    switch (i+1) {
      case 1: etiqueta="Voltaje"; break;
      case 2: etiqueta="Amperaje"; break;
      case 3: etiqueta="Potencia"; break;
      case 4: etiqueta="Temperatura"; break;
      case 5: etiqueta="Inductancia"; break;
      case 6: etiqueta="Capacitancia"; break;
    }
    text(etiqueta, x + btnAncho/2, y + btnAlto/2);
  }

  // --- Cuadros de valores ---
  mostrarValores();

  // --- Gráficos ---
  int xg = 300;
  graficarVariable(xg, 100, 400, 120, histVolt, 0, 50, voltaje, "Voltaje (V)", color(255, 100, 0));
  graficarVariable(xg, 240, 400, 120, histAmp, 0, 20, amperaje, "Amperaje (A)", color(0, 150, 255));
  graficarVariable(xg, 380, 400, 120, histPot, 0, 250, potencia, "Potencia (W)", color(255, 0, 150));
  graficarVariable(xg, 520, 400, 120, histTemp, 0, 50, temperatura, "Temperatura (°C)", color(255, 0, 0));
}

// --------------------------------------------------------------------
// Muestra los valores actuales en cuadros a la derecha de los botones
// --------------------------------------------------------------------
void mostrarValores() {
  for (int i = 0; i < numBotones; i++) {
    int x = 135;
    int y = 100 + i * (btnAlto + espaciado);
    fill(240);
    rect(x, y, btnAncho, btnAlto, 10);
    fill(0);
    String texto = "";
    switch (i+1) {
      case 1: texto = nf(voltaje, 1, 2) + " V"; break;
      case 2: texto = nf(amperaje, 1, 2) + " A"; break;
      case 3: texto = nf(potencia, 1, 1) + " W"; break;
      case 4: texto = nf(temperatura, 1, 1) + " °C"; break;
      case 5: texto = "Inductancia H"; break;
      case 6: texto = "Capacitancia uF"; break;
    }
    textAlign(CENTER, CENTER);
    text(texto, x + btnAncho/2, y + btnAlto/2);
  }
}

// --------------------------------------------------------------------
// Envío de comandos por botones
// --------------------------------------------------------------------
void mousePressed() {
  String codigo = "";
  for (int i = 0; i < numBotones; i++) {
    int x = 20;
    int y = 100 + i * (btnAlto + espaciado);
    if (mouseX > x && mouseX < x + btnAncho && mouseY > y && mouseY < y + btnAlto) {
      switch (i+1) {
        case 1: codigo="V1"; break;
        case 2: codigo="A1"; break;
        case 3: codigo="P1"; break;
        case 4: codigo="T1"; break;
        case 5: codigo="I1"; break;
        case 6: codigo="C1"; break;
      }
      myPort.write(codigo);
      println("Enviado: " + codigo);
    }
  }
}

// --------------------------------------------------------------------
// Procesa datos recibidos desde Arduino
// --------------------------------------------------------------------
void serialEvent(Serial p) {
  String lectura = p.readStringUntil('\n');
  if (lectura != null) {
    lectura = lectura.trim();
    println("Recibido: " + lectura);
    
    try {
      if (lectura.indexOf('V') != -1) voltaje = extraerValor(lectura, 'V');
      if (lectura.indexOf('A') != -1) amperaje = extraerValor(lectura, 'A');
      if (lectura.indexOf('P') != -1) potencia = extraerValor(lectura, 'P');
      if (lectura.indexOf('T') != -1) temperatura = extraerValor(lectura, 'T');
      
      actualizarHistorial(histVolt, voltaje);
      actualizarHistorial(histAmp, amperaje);
      actualizarHistorial(histPot, potencia);
      actualizarHistorial(histTemp, temperatura);
    }
    catch (Exception e) {
      println("Error al procesar: " + e);
    }
  }
}

// --------------------------------------------------------------------
// Extrae el valor que sigue a una letra clave (V, A, P, T)
// --------------------------------------------------------------------
float extraerValor(String s, char clave) {
  int start = s.indexOf(clave);
  if (start == -1) return 0;
  int end = s.indexOf(',', start);
  String num = (end != -1) ? s.substring(start+1, end) : s.substring(start+1);
  return float(trim(num));
}

// --------------------------------------------------------------------
// Actualiza el historial desplazando valores
// --------------------------------------------------------------------
void actualizarHistorial(float[] hist, float nuevo) {
  for (int i = 0; i < hist.length - 1; i++) {
    hist[i] = hist[i+1];
  }
  hist[hist.length - 1] = nuevo;
}

// --------------------------------------------------------------------
// Dibuja gráfico genérico para cualquier variable
// --------------------------------------------------------------------
void graficarVariable(int x0, int y0, int ancho, int alto,
                      float[] datos, float minVal, float maxVal,
                      float actual, String titulo, color c) {
  // Fondo
  fill(255);
  stroke(0);
  rect(x0, y0, ancho, alto);
  
  fill(0);
  textAlign(LEFT, TOP);
  text(titulo, x0 + 10, y0 + 5);
  
  // Escalas cada 25% del rango
  stroke(180);
  for (int i = 0; i <= 4; i++) {
    float v = map(i, 0, 4, minVal, maxVal);
    float y = map(v, minVal, maxVal, y0 + alto - 20, y0 + 20);
    line(x0 + 50, y, x0 + ancho - 10, y);
    fill(0);
    textAlign(RIGHT, CENTER);
    text(nf(v, 1, 1), x0 + 45, y);
  }
  
  // Curva
  stroke(c);
  noFill();
  beginShape();
  for (int i = 0; i < datos.length; i++) {
    float x = map(i, 0, datos.length-1, x0 + 55, x0 + ancho - 10);
    float y = map(datos[i], minVal, maxVal, y0 + alto - 20, y0 + 20);
    vertex(x, y);
  }
  endShape();
  
  // Valor actual
  stroke(0, 0, 255);
  float yAct = map(actual, minVal, maxVal, y0 + alto - 20, y0 + 20);
  line(x0 + 50, yAct, x0 + ancho - 10, yAct);
  fill(0, 0, 255);
  textAlign(LEFT, CENTER);
  text("Actual: " + nf(actual, 1, 2), x0 + 60, yAct - 15);
}
