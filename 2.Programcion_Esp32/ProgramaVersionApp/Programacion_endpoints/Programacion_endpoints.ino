#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_AP";
const char* password = "123456789";

// Crear una instancia del servidor web
WebServer server(80);

// Definir el pin de salida para el MOSFET
const int mosfetPin = 18;         // Pin para controlar el MOSFET (GPIO18)

// Variables para guardar el estado del sistema
String mode = "uA";               // Modo actual (uA, mA, Auto)
float currentValue = 0.0;         // Valor de corriente leído
String range = "µA";              // Escala de medición actual (uA o mA)

// Función para leer la corriente sin conversión
float readCurrent() {
  int sensorValue = analogRead(34);  // Pin 34 como entrada de señal analógica
  float voltage = (sensorValue / 4095.0) * 500.0;  // Convertir a milivoltios (0-3300mV)
  return voltage;
}

// Función para procesar el modo automático
void handleAutoMode(float currentValue) {
  if (currentValue >= 495) {
    // Activar el MOSFET cuando la corriente supere los 495mV
    digitalWrite(mosfetPin, HIGH);
    range = "mA";  // Cambiar la escala a miliamperios cuando el MOSFET esté encendido
  } else if (currentValue < 1) {
    // Apagar el MOSFET cuando la corriente sea inferior a 1mV
    digitalWrite(mosfetPin, LOW);
    range = "µA";  // Cambiar la escala a microamperios cuando el MOSFET esté apagado
  }
}

// Función para manejar el cambio de modo desde la aplicación
void handleSetMode() {
  if (server.hasArg("mode")) {
    mode = server.arg("mode");

    // Lógica para los modos
    if (mode == "mA") {
      digitalWrite(mosfetPin, HIGH);  // Encender MOSFET en mA
      range = "mA";                   // Fijar el rango en miliamperios
    } else if (mode == "uA") {
      digitalWrite(mosfetPin, LOW);   // Apagar MOSFET en uA
      range = "µA";                   // Fijar el rango en microamperios
    }
    // No encendemos/apagamos directamente en modo Auto, lo hace handleAutoMode()
  }

  // Responder a la aplicación confirmando el cambio de modo
  server.send(200, "text/plain", "Mode changed to " + mode);
}

// Función para devolver los datos actuales en formato JSON
void handleData() {
  currentValue = readCurrent();  // Leer la corriente

  // Si el modo es "Auto", procesar automáticamente y cambiar la escala
  if (mode == "Auto") {
    handleAutoMode(currentValue);
  }

  // Crear el JSON con las variables que queremos enviar
  String jsonData = "{";
  jsonData += "\"current\": " + String(currentValue, 2) + ",";  // Agregar lectura de corriente
  jsonData += "\"mode\": \"" + mode + "\",";  // Enviar el modo actual
  jsonData += "\"range\": \"" + range + "\"";  // Enviar la escala actual (mA o µA)
  jsonData += "}";

  // Enviar los datos en formato JSON
  server.send(200, "application/json", jsonData);
}

void setup() {
  Serial.begin(115200);
  
  // Configurar los pines
  pinMode(34, INPUT);  // Pin 34 para la señal de corriente
  pinMode(mosfetPin, OUTPUT);  // Pin para el MOSFET

  // Configurar el WiFi
  WiFi.softAP(ssid, password);
  Serial.println("Punto de acceso iniciado");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.softAPIP());

  // Configurar las rutas del servidor
  server.on("/setMode", handleSetMode);   // Endpoint para cambiar el modo
  server.on("/data", handleData);         // Endpoint para obtener los datos actuales
  server.begin();                         // Iniciar el servidor
}

void loop() {
  server.handleClient();  // Manejar las solicitudes del cliente
}
