#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_AP";
const char* password = "123456789";

// Crear una instancia del servidor web
WebServer server(80);

// Definir los pines de entrada
const int pinAutoInput = 19;      // Pin para la entrada Auto (GPIO19)
const int pinRangeInput = 5;      // Pin para la entrada de rango (uA/mA) (GPIO5)
const int inputPin = 34;          // Pin 34 como entrada de señal analógica

// Función para leer la corriente sin conversión
float readCurrent() {
  int sensorValue = analogRead(inputPin);
  float voltage = (sensorValue / 4095.0) * 3300.0;  // Convertir a milivoltios (0-3300mV)
  return voltage;
}

// Función para la página principal
void handleRoot() {
  // Leer los estados lógicos de las entradas
  bool isAuto = digitalRead(pinAutoInput);
  bool isMilliAmp = digitalRead(pinRangeInput);  // HIGH = mA, LOW = uA
  
  // Definir la unidad de medida dependiendo del estado del pin de rango
  String displayUnit = "";
  if (isAuto) {
    displayUnit = "Auto";
  } else if (isMilliAmp) {
    displayUnit = "mA";
  } else {
    displayUnit = "µA";
  }
  
  float currentValue = readCurrent();  // Leer la corriente
  
  // Crear el contenido HTML
  String html = "<html><head>";
  html += "<meta charset='UTF-8'>";  // Añadir la codificación UTF-8
  html += "<style>";
  html += "html, body { height: 100%; margin: 0; font-family: Arial; font-size: 1.6em; background-color: #87CEEB; }"; // Fondo celeste
  html += "body { display: flex; justify-content: center; align-items: center; }";
  html += ".container { text-align: center; }";
  html += ".box { border: 1px solid black; display: inline-block; padding: 20px; margin-bottom: 20px; border-radius: 15px; font-size: 1.6em; }";
  html += ".button { padding: 30px 60px; font-size: 1.6em; margin: 10px; background-color: grey; color: white; border: none; cursor: pointer; border-radius: 8px; }";
  html += ".button.selected { background-color: white; color: black; }";  // Botón seleccionado en blanco
  html += ".small-text { font-size: 0.8em; color: #333; }";
  html += "</style>";
  
  // Añadir JavaScript para auto-actualización
  html += "<script>";
  html += "setInterval(function() { window.location.reload(); }, 1000);";  // Refrescar cada 1 segundo
  html += "</script>";
  
  html += "</head><body>";
  
  html += "<div class='container'>";
  
  html += "<div class='box'><h1>Lectura de Corriente:</h1>";
  html += "<h2>" + String(currentValue, 2) + " " + displayUnit + "</h2></div>";
  
  html += "<h3>Selector de medicion</h3>";
  
  // Botones con estado visual según las entradas
  html += "<input class='button " + String((isAuto) ? "selected" : "") + "' type='button' value='Auto'>";
  html += "<input class='button " + String((!isAuto && isMilliAmp) ? "selected" : "") + "' type='button' value='mA'>";
  html += "<input class='button " + String((!isAuto && !isMilliAmp) ? "selected" : "") + "' type='button' value='µA'>";
  
  html += "<p class='small-text'>Rangos:<br>0-500 µA<br>0-500 mA</p>";
  html += "</div>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  pinMode(inputPin, INPUT);
  pinMode(pinAutoInput, INPUT);
  pinMode(pinRangeInput, INPUT);

  WiFi.softAP(ssid, password);
  Serial.println("Punto de acceso iniciado");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();}
   if (WiFi.status() == WL_CONNECTED) {
    esp_sleep_enable_timer_wakeup(5000000);  // Configurar para despertar cada 5 segundos
    esp_light_sleep_start();                 // Entrar en light sleep
  }
}
