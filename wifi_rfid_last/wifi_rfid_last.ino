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

const char* ssid = "ha";
const char* password = "123456789";

const int LED_G = D8;
const int LED_R = D2;
const int pinRST = 5; 
const int pinSDA = 4;
const int stockage_max = 10;
const int decalage = -5; 

int UID [stockage_max][4];
time_t date_et_heure[stockage_max];
int nb_de_donnees = 0;
IPAddress server(203, 234, 19, 82);
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

  //led
  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  
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

void Auth_write(MFRC522 rfid, byte sector, byte * data, MFRC522::MIFARE_Key keyA, MFRC522::MIFARE_Key keyB){
  
    byte blockAddr = sector * 4;
    byte trailerBlock = blockAddr + 3;
    
    if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &keyA, &(rfid.uid)) != MFRC522::STATUS_OK) {
      Serial.println("인증 실패1A");
      return;
    }
    Serial.println("인증 성공1A");

    if(rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &keyB, &(rfid.uid)) != MFRC522::STATUS_OK){
      Serial.println("인증 실패1B");
      return;
    }
    Serial.println("인증 성공1B");
    if (rfid.MIFARE_Write(blockAddr, data, 16) == 0) {
          Serial.println("값 쓰기 성공1B");
      } else {
          Serial.println("값 쓰기 실패1B");
        }

      if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &keyA, &(rfid.uid)) != MFRC522::STATUS_OK) {
        Serial.println("인증 실패2A");
        return;
      }
      Serial.println("인증 성공2A");
      if(rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &keyB, &(rfid.uid)) != MFRC522::STATUS_OK){
        Serial.println("인증 실패2B");
        return;
      }
      Serial.println("인증 성공B");
      if (rfid.MIFARE_Write(blockAddr+1, &data[16], 16) == 0) {
            Serial.println("값 쓰기 성공2B");
        } else {
            Serial.println("값 쓰기 실패2B");
          }
       
    delay(100);

    return;
  }
/*
void Auth_read(MFRC522 rfid, byte sector, byte * read_data, MFRC522::MIFARE_Key keyA, MFRC522::MIFARE_Key keyB){
  byte blockAddr = sector * 4;
  byte trailerBlock = blockAddr + 3;
  
  if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &keyA, &(rfid.uid)) != MFRC522::STATUS_OK) {
      Serial.println("인증 실패1A");
      return;
    }
    Serial.println("인증 성공1A");

    if(rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &keyB, &(rfid.uid)) != MFRC522::STATUS_OK){
      Serial.println("인증 실패1B");
      return;
    }
    Serial.println("인증 성공1B");
    if (rfid.MIFARE_Read(blockAddr, read_data, &buffersize) == 0) {
          Serial.println("값 읽기 성공1B");
      } else {
          Serial.println("값 읽기 실패1B");
        }

      if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &keyA, &(rfid.uid)) != MFRC522::STATUS_OK) {
        Serial.println("인증 실패2A");
        return;
      }
      Serial.println("인증 성공2A");
      if(rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &keyB, &(rfid.uid)) != MFRC522::STATUS_OK){
        Serial.println("인증 실패2B");
        return;
      }
      Serial.println("인증 성공2B");
      
      if (rfid.MIFARE_Read(blockAddr+1, read_data2, &buffersize) == 0) {
            Serial.println("값 읽기 성공2B");
        } else {
            Serial.println("값 읽기 실패2B");
          }

    return;
}
*/
void loop()
{
  if (rfid.PICC_IsNewCardPresent()) 
  {
    if (rfid.PICC_ReadCardSerial()) 
    {
      for (int i = 0; i <= 3; i++) {
         Serial.print(rfid.uid.uidByte[i]);
      }
       Serial.println("");

      String hexString = "706bfadfbe6c2e8e0d8daf9a78325cce388193066f2f593216a52ce8664886d0";
      int numBytes = hexString.length() / 2; 
      byte hexBytes[numBytes]; 
      byte data[32];
      //byte data[] = {0x70, 0x6b, 0xfa, 0xdf, 0xbe, 0x6c, 0x2e, 0x8e, 0x0d, 0x8d, 0xaf, 0x9a, 0x78, 0x32, 0x5c, 0xce, 0x38, 0x81, 0x93, 0x06, 0x6f, 0x2f, 0x59, 0x32, 0x16, 0xa5, 0x2c, 0xe8, 0x66, 0x48, 0x86, 0xd0};
      byte zero_data[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
      byte read_data[18];
      byte read_data2[18];
      Serial.print("읽기전 데이터: ");
      for (byte i = 0; i < 16; i++) {
        Serial.print(read_data[i], HEX);
        Serial.print(" ");
      }
      for (int i = 0; i < numBytes; i++) {
        hexBytes[i] = (byte) strtol(hexString.substring(i * 2, i * 2 + 2).c_str(), NULL, 16);
      }

      MFRC522::MIFARE_Key keyA = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
      MFRC522::MIFARE_Key keyB = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
      
    byte sector = 5;
      //Auth_write(sector, data, keyA, keyB);


    byte blockAddr = sector * 4;
    byte trailerBlock = blockAddr + 3;
    byte buffersize = sizeof(read_data);
    
    if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &keyA, &(rfid.uid)) != MFRC522::STATUS_OK) {
      Serial.println("인증 실패1A");
      return;
    }
    Serial.println("인증 성공1A");

    if(rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &keyB, &(rfid.uid)) != MFRC522::STATUS_OK){
      Serial.println("인증 실패1B");
      return;
    }
    Serial.println("인증 성공1B");
    if (rfid.MIFARE_Read(blockAddr, read_data, &buffersize) == 0) {
          Serial.println("값 읽기 성공1B");
      } else {
          Serial.println("값 읽기 실패1B");
        }

      if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &keyA, &(rfid.uid)) != MFRC522::STATUS_OK) {
        Serial.println("인증 실패2A");
        return;
      }
      Serial.println("인증 성공2A");
      if(rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &keyB, &(rfid.uid)) != MFRC522::STATUS_OK){
        Serial.println("인증 실패2B");
        return;
      }
      Serial.println("인증 성공2B");
      
      if (rfid.MIFARE_Read(blockAddr+1, read_data2, &buffersize) == 0) {
            Serial.println("값 읽기 성공2B");
        } else {
            Serial.println("값 읽기 실패2B");
          }

      Serial.println();
      
      memcpy(data, read_data, 16);
      memcpy(data + 16, read_data2, 16);
      for (byte i = 0; i < 32; i++) {
        Serial.print(data[i], HEX);
        Serial.print(" ");
      }

      // clinet tcp connect 
      if (!client.connect(server, port)) {
      Serial.println("Connection failed.");
      return;
      }
      int dataSize = strlen((char*)data);
      byte uidByte[4];
      byte recv_data[32];
      for (byte i = 0; i < 4; i++) {
        uidByte[i] = rfid.uid.uidByte[i];
        Serial.print(uidByte[i]);
      }
      byte send_data[36];
      int money_data;
      int send_datasize = strlen((char*)send_data);
      memcpy(send_data, uidByte, 4);
      memcpy(send_data+4, data, 32);
      //client.write(uidByte, 4);
      //client.write(data, dataSize);
      Serial.println("send data:  ");
      for (byte i = 0; i < 36; i++) {
        uidByte[i] = rfid.uid.uidByte[i];
        Serial.print(send_data[i], HEX);
      }

      // money data
      money_data = 1000;
     
      client.write(send_data, 36);
      client.println(money_data);
      client.stop();

      byte ok_data[2] = {0x6F, 0x6B};//ok
      byte fa_data[2] = {0x66, 0x61};//fa
      int cmp;
      bool isEqual = true;
      int len = client.read(recv_data, sizeof(recv_data));
      Serial.println();
      if (len > 0) {
        
        for(int i=0; i<2; i++) {
          if(fa_data[i] != recv_data[i]) {
            isEqual = false;
            break;
          }
        }
   
        Serial.println();
        Serial.println("recv data: ");
        for (byte i = 0; i < 32; i++) {
        Serial.print(recv_data[i], HEX);
        Serial.print(" ");
      }
        if(isEqual == false){
          // nomal card
          // to do green led 출력 및 새로운 해시 저장하는 거 만들기
          
          Serial.println("ok_data");
          Auth_write(rfid, sector, recv_data,keyA,keyB);
          digitalWrite(LED_G, HIGH);
          delay(1000);
          digitalWrite(LED_G, LOW);
          
        }else{
          // detect replicated card
          // to do red led 출력 
          digitalWrite(LED_R, HIGH);
          delay(1000);
          digitalWrite(LED_R, LOW);
          
          Serial.println("fa_data");
        }
      } else if (len == 0) {
        // 데이터가 없음
        Serial.println("no data");
      } else {
        // 에러 발생
        Serial.println("recv error");
      }

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      client.stop();
      delay(1000);
    }
  }


}
