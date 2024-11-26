/*
 * SENDOR DE GAS Y RELE. CONTROLADOS A TRAVEZ DE UNA RED WIFI
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Configuración WiFi
const char* ssid = "RICHARD"; //
const char* password = "TECNICA123";

// Pines
const int gasSensorPin = A0;  // Pin analógico para el sensor de gas
const int relayPin = D1;      // Pin digital para el relé

// Variables
float gasThreshold = 40.0;   // Umbral de gas en porcentaje
bool relayState = false;     // Estado actual del relé

// Servidor web
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Configuración del sensor y relé
  pinMode(gasSensorPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Relé inicialmente apagado

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");
  Serial.println(WiFi.localIP());

  // Configuración del servidor web
  server.on("/", handleRoot);
  server.on("/relay/on", handleRelayOn);
  server.on("/relay/off", handleRelayOff);
  server.on("/set-threshold", handleSetThreshold);

  server.begin();
  Serial.println("Servidor web iniciado");
}

void loop() {
  server.handleClient();

  // Monitoreo del sensor de gas
  int rawGasLevel = analogRead(gasSensorPin);
  float gasPercentage = map(rawGasLevel, 0, 1023, 0, 100); // Convertir a porcentaje
  Serial.println("Nivel de gas (%): " + String(gasPercentage));

  // Control automático del relé basado en el umbral
  if (gasPercentage > gasThreshold) {
    relayState = true;
    digitalWrite(relayPin, HIGH);
  } else if (!relayState) {
    digitalWrite(relayPin, LOW);
  }

  delay(1000);
}

// Manejo de la página principal
void handleRoot() {
  int rawGasLevel = analogRead(gasSensorPin);
  float gasPercentage = map(rawGasLevel, 0, 1023, 0, 100); // Convertir a porcentaje
  String html = "<html>\
                  <head><title>Control Sensor de Gas</title></head>\
                  <body >\
                  <center><h1>Monitoreo del Sensor de Gas</h1></center>\
                  <p>Nivel de gas: " + String(gasPercentage) + "%</p>\
                  <p>Estado del rele: " + String(relayState ? "Encendido" : "Apagado") + "</p>\
                  <p>Umbral actual: " + String(gasThreshold) + "%</p>\
                  <button><a href='/relay/on'>Encender Rele</a></button><br>\
                  <a href='/relay/off'>Apagar Rele</a><br>\
                  <form action='/set-threshold' method='get'>\
                    <label for='threshold'>Establecer nuevo umbral (%):</label>\
                    <input type='number' id='threshold' name='threshold' step='1'>\
                    <input type='submit' value='Actualizar'>\
                  </form>\
                  </body>\
                  </html>";
  server.send(200, "text/html", html);
}

// Manejo para encender el relé
void handleRelayOn() {
  relayState = true;
  digitalWrite(relayPin, HIGH);
  server.send(200, "text/plain", "Relé encendido");
}

// Manejo para apagar el relé
void handleRelayOff() {
  relayState = false;
  digitalWrite(relayPin, LOW);
  server.send(200, "text/plain", "Relé apagado");
}

// Manejo para actualizar el umbral
void handleSetThreshold() {
  if (server.hasArg("threshold")) {
    gasThreshold = server.arg("threshold").toFloat();
    server.send(200, "text/plain", "Umbral actualizado a " + String(gasThreshold) + "%");
  } else {
    server.send(400, "text/plain", "Parámetro 'threshold' no proporcionado");
  }
}
