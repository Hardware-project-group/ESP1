#include "wifi_setup.h"
#include "Display.h"

const char *ssid = "MSI5554";
const char *pw = "12345678";

const char* ssid2     = "ESP32-Access-Point";
const char* password = "123456789";


int count = 0;

void connectWiFi(){
  WiFi.mode(WIFI_OFF);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pw);
  Serial.println("Connecting to Wifi");
  lcd.clear();
  lcd.println("Connecting to Wifi");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println("Connecting to wifi");
    if(count == 10){
      WiFi.softAP(ssid2, password);
      IPAddress IP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(IP);
      nowifi = true;
      break;
    }
    count++;
  }


  Serial.println("Connected to ");
  Serial.println(ssid);
}
