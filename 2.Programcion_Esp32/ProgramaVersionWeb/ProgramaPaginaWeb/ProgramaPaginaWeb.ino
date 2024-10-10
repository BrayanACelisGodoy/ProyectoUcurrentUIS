#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_AP";
const char* password = "123456789";

// Crear una instancia del servidor web
WebServer server(80);

// Definir los pines de entrada y salida
const int pinAutoInput = 33;
const int pinAuto = 26;
const int pinMilliAmp = 27;
const int pinMicroAmp = 25;
const int inputPin = 34;

// Definir variables para almacenamiento de muestras
const int sampleInterval = 1000; // Intervalo de muestreo de 1 segundo
const int totalSamples = 30;     // Tomar muestras durante 30 segundos
float samples[totalSamples];
int sampleIndex = 0;
bool collectingSamples = false;

// Función para leer la corriente (sin conversión)
float readCurrent() {
  int sensorValue = analogRead(inputPin);  // Leer valor analógico
  float voltage = (sensorValue / 4095.0) * 500.0;  // Convertir a milivoltios (0-500mV)
  return voltage;
}

// Función para manejar la activación de los pines según el botón seleccionado
void handlePinActivation(String unit) {
  digitalWrite(pinAuto, LOW);
  digitalWrite(pinMilliAmp, LOW);
  digitalWrite(pinMicroAmp, LOW);
  
  if (unit == "auto") {
    digitalWrite(pinAuto, HIGH);
  } else if (unit == "mA") {
    digitalWrite(pinMilliAmp, HIGH);
  } else if (unit == "uA") {
    digitalWrite(pinMicroAmp, HIGH);
  }
}

// Función para la página principal
void handleRoot() {
  String unit = server.arg("unit");
  if (unit == "") unit = "auto";  // Valor por defecto
  
  handlePinActivation(unit);  // Activar el pin correspondiente
  
  float currentValue = readCurrent();  // Leer la corriente
  
  String displayUnit = (unit == "uA") ? "µA" : "mA";  // Definir la unidad para mostrar
  
  // Crear el contenido HTML
  String html = "<html><head>";
  html += "<meta charset='UTF-8'>";  // Añadir la codificación UTF-8
  html += "<style>";
  html += "html, body { height: 100%; margin: 0; overflow: hidden; font-family: Arial; font-size: 1.6em; }";
  html += "body { display: flex; justify-content: center; align-items: center; background-color: #87CEEB; }";
  html += ".container { text-align: center; }";
  html += ".box { border: 1px solid black; display: inline-block; padding: 20px; margin-bottom: 20px; border-radius: 15px; font-size: 1.6em; }";
  html += ".button { padding: 30px 60px; font-size: 1.6em; margin: 10px; background-color: grey; color: white; border: none; cursor: pointer; border-radius: 8px; }";
  html += ".button.selected { background-color: white; color: black; }";
  html += ".small-text { font-size: 0.8em; color: #333; }";
  html += "</style>";
  html += "</head><body>";
  
  html += "<div class='container'>";
  
  html += "<div class='box'><h1>Lectura de Corriente:</h1>";
  html += "<h2>" + String(currentValue, 2) + " " + displayUnit + "</h2></div>";
  
  html += "<h3>Selector de medicion</h3>";
  
  html += "<form action='/' method='GET'>";
  html += "<input class='button " + String((unit == "auto") ? "selected" : "") + "' type='submit' name='unit' value='auto'>";
  html += "<input class='button " + String((unit == "mA") ? "selected" : "") + "' type='submit' name='unit' value='mA'>";
  html += "<input class='button " + String((unit == "uA") ? "selected" : "") + "' type='submit' name='unit' value='uA'>";
  html += "</form>";
  
  // Agregar botón para ver el gráfico de los datos sensados
  html += "<form action='/graph' method='GET'>";
  html += "<input class='button' type='submit' value='Gráfico'>";
  html += "</form>";
  
  html += "<p class='small-text'>Rangos:<br>0-500 µA<br>0-500 mA</p>";
  html += "</div>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Función para mostrar el gráfico de datos sensados
void handleGraphPage() {
  String html = "<html><head>";
  html += "<meta charset='UTF-8'>";  // Añadir la codificación UTF-8
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";  // Incluir Chart.js desde CDN
  html += "<style>";
  html += "body { font-family: Arial; font-size: 1.2em; text-align: center; }";
  html += "</style></head><body>";
  html += "<h1>Gráfico de valores durante 30 segundos</h1>";
  html += "<canvas id='myChart' width='400' height='200'></canvas>";
  html += "<script>";
  html += "const ctx = document.getElementById('myChart').getContext('2d');";
  html += "const myChart = new Chart(ctx, {";
  html += "type: 'line',";
  html += "data: {";
  html += "labels: [";

  // Añadir etiquetas de tiempo (segundos)
  for (int i = 0; i < totalSamples; i++) {
    html += "'" + String(i) + "s'";
    if (i < totalSamples - 1) html += ",";
  }

  html += "], datasets: [{";
  html += "label: 'Corriente en mV',";
  html += "data: [";

  // Añadir los datos de las muestras
  for (int i = 0; i < totalSamples; i++) {
    html += String(samples[i], 2);
    if (i < totalSamples - 1) html += ",";
  }

  html += "], borderColor: 'rgba(75, 192, 192, 1)', borderWidth: 2}]},";
  html += "options: { scales: { y: { beginAtZero: true } } }";
  html += "});";
  html += "</script>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Función para tomar muestras
void takeSample() {
  if (collectingSamples && sampleIndex < totalSamples) {
    samples[sampleIndex] = readCurrent();  // Tomar la muestra
    sampleIndex++;
    
    if (sampleIndex == totalSamples) {
      collectingSamples = false;  // Detener la recolección de muestras
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(inputPin, INPUT);
  pinMode(pinAuto, OUTPUT);
  pinMode(pinMilliAmp, OUTPUT);
  pinMode(pinMicroAmp, OUTPUT);
  pinMode(pinAutoInput, INPUT);

  WiFi.softAP(ssid, password);
  Serial.println("Punto de acceso iniciado");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/graph", handleGraphPage);
  server.begin();
  
  // Iniciar la recolección de muestras
  collectingSamples = true;
  sampleIndex = 0;
}

void loop() {
  server.handleClient();
  takeSample();  // Tomar una muestra cada intervalo
  delay(sampleInterval);  // Esperar antes de la próxima muestra
}
