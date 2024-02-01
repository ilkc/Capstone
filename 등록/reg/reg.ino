#include <SPI.h>
#include <MFRC522.h>
#include <WiFiClient.h>
#if defined ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined ARDUINO_ARCH_ESP32
#include "WiFi.h"
#include <WebServer.h>
#endif

#include <WiFiClient.h>
#include <time.h>

const char* ssid = "MOAT_WIFI";
const char* password = "minsookim";

const int pinRST = 5; 
const int pinSDA = 4;
const int stockage_max = 10;
const int decalage = -5; 

int UID [stockage_max][4];
time_t date_et_heure[stockage_max];
int nb_de_donnees = 0;
IPAddress server(192, 168, 0, 3);
int port = 9876;
WiFiClient client;
MFRC522 rfid(pinSDA, pinRST);

#if defined ARDUINO_ARCH_ESP8266 
ESP8266WebServer serveur(80);
#elif defined ARDUINO_ARCH_ESP32 
#endif

void setup()
{
  SPI.begin();
  rfid.PCD_Init();
  Serial.begin(9600);

  Serial.println();

  WiFi.begin(ssid, password);
  Serial.print("Connexion au reseau WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println();

  Serial.print("Nom du reseau WiFi: ");
  Serial.println(ssid);
  Serial.print("Adresse IP de la page web: ");
  Serial.println(WiFi.localIP());

  configTime(decalage * 3600, 0, "ca.pool.ntp.org"); 

  Serial.print("Attente date et heure");

  while (time(nullptr) <= 100000) {
    Serial.print(".");
    delay(1000);
  }
  
  Serial.println();
  Serial.println("Pret.");

  
}

void loop() {
  // put your main code here, to run repeatedly:

}
