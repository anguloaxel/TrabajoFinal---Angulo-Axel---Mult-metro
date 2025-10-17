import processing.serial.*;

Serial myPort;

float temperatura = 0;
float voltaje = 0;
float corriente = 0;

int N = 10;
float[] tempBuffer = new float[N];
float[] voltBuffer = new float[N];
float[] corrBuffer = new float[N];
int avgIndex = 0;

int ancho = 900;
int alto = 600;
int puntosMax = 300;

float[] tempHist = new float[puntosMax];
float[] voltHist = new float[puntosMax];
float[] corrHist = new float[puntosMax];
int index = 0;

void settings() {
  size(ancho, alto);
}

void setup() {
  surface.setTitle("Monitor Arduino - Temperatura / Voltaje / Corriente");

  println("Puertos disponibles:");
  println(Serial.list());
  // --- Usa el primer puerto disponible (como antes) ---
  myPort = new Serial(this, Serial.list()[2], 9600);
  myPort.bufferUntil('\n');
}

void draw() {
  for (int i = 0; i < height; i++) {
    stroke(lerpColor(color(0, 30, 50), color(0, 0, 0), map(i, 0, height, 0, 1)));
    line(0, i, width, i);
  }

  stroke(60);
  for (int i = 0; i < ancho; i += 50) line(i, 0, i, alto);
  for (int j = 0; j < alto; j += 50) line(0, j, ancho, j);

  strokeWeight(2);
  drawSmoothGraph(tempHist, color(255, 80, 80));
  drawSmoothGraph(voltHist, color(100, 255, 100));
  drawSmoothGraph(corrHist, color(80, 150, 255));

  fill(255);
  textSize(18);
  text("Temperatura: " + nf(temperatura, 1, 1) + " °C", 20, 30);
  text("Voltaje: " + nf(voltaje, 1, 2) + " V", 20, 55);
  text("Corriente: " + nf(corriente, 1, 3) + " A", 20, 80);
}

void serialEvent(Serial myPort) {
  String inString = myPort.readStringUntil('\n');
  if (inString != null) {
    inString = trim(inString);
    String[] datos = split(inString, ',');
    if (datos.length == 3) {
      temperatura = smoothValue(float(datos[0]), tempBuffer);
      voltaje = smoothValue(float(datos[1]), voltBuffer);
      corriente = smoothValue(float(datos[2]), corrBuffer);

      tempHist[index] = map(temperatura, 0, 50, height - 50, 100);
      voltHist[index] = map(voltaje, 0, 25, height - 50, 300);
      corrHist[index] = map(corriente, 0, 5, height - 50, 500);

      index = (index + 1) % puntosMax;
    }
  }
}

float smoothValue(float nuevo, float[] buffer) {
  buffer[avgIndex % N] = nuevo;
  float suma = 0;
  for (float v : buffer) suma += v;
  avgIndex++;
  return suma / N;
}

void drawSmoothGraph(float[] valores, color c) {
  stroke(c);
  noFill();
  beginShape();
  for (int i = 0; i < puntosMax; i++) {
    float x = map(i, 0, puntosMax, 0, width);
    curveVertex(x, valores[(i + index) % puntosMax]);
  }
  endShape();
}
