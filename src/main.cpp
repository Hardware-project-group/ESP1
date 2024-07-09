//Global Section
#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <WiFi.h>
#include <Keypad.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <RTClib.h>
#include <SPI.h>


//local section
#include "Finger_check.h"
#include "enroll.h"
#include "wifi_setup.h"
#include "keyCode.h"
#include "Display.h"
#include "DoorControl.h"

HardwareSerial serialPort(2); 

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&serialPort);
RTC_DS3231 rtc;
HTTPClient http;
WebServer server(80);
WiFiServer server2(80);

uint8_t id;
uint8_t fingerId;
bool passcodeVerified = false;
String passcode = "";
String postdata = "id=";
String postdataIP;
char dateBuffer[20];
String header;



//Door Open System
int motor1Pin1 = 15; 
int motor1Pin2 = 2; 
int enable1Pin = 4;
int doorsensor = 13;
int doorsensor1 = 18;

const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;

const int irSensor = 35;
const int ledPin = 32;
int irReading;
bool nowifi = false;

int personCount=0;

void updateDis();

void doorstate(){
  String state = "Close";
  int doorsensorstate = digitalRead(doorsensor);
  if(doorsensorstate == LOW){
    state = "Close";
  }else{
    state = "Open";
  }
  http.begin("http://192.168.137.1:5000/SendDoorState");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  postdata = "Door" + state;
  int httpResponseCode = http.POST(postdata);
  Serial.println("Door state send http response code: ");
  Serial.println(httpResponseCode);
  http.end();
}

void getTime(){
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  // Print current date and time
  DateTime now = rtc.now();
  sprintf(dateBuffer, "%04d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  // Serial.print("Current Date and Time: ");
  // Serial.println(dateBuffer);
  lcd.setCursor(0,0);
  lcd.print(dateBuffer);
}

void GetpersonCount(){ 
  http.begin("http://192.168.137.1:5000/getPersonCount");
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println(payload); 
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        personCount = doc["personCount"];
        Serial.print("Person Count: ");
        Serial.println(personCount);
      } else {
        Serial.print("JSON deserialization error: ");
        Serial.println(error.c_str());
      }
  }
}

void handleFingerprint() {
  Serial.println("Handling fingerprint request...");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  http.begin("http://192.168.137.1:5000/update-outside-finger");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  

  if (server.hasArg("fingerID")) {
    lcd.clear();
    lcd.print("Start Enrolling Fingerprint");
    String paramValueStr = server.arg("fingerID");
    int paramValue = paramValueStr.toInt();
    // Process the parameter value (e.g., print to Serial)
    Serial.print("Received parameter value: ");
    Serial.println(paramValue);

    int enrollStatus = -1;
    while (enrollStatus != 1) {
      // Attempt to enroll the fingerprint
      lcd.clear();
      lcd.print("Enrolling fingerprint...");
      enrollStatus = getFingerprintEnroll(paramValue);
      if (enrollStatus != 1) {
        lcd.print("Something went wrong..Try again!");
      }
      delay(2000); // Add a small delay between enrollment attempts
    }

    // Send a response to the client
    server.send(200, "text/plain", "FingerPrint enroll successfully!");
    String value  = String(paramValue);
    Serial.print(value);
    postdata = postdata + value;
    Serial.print("POST data: ");
    Serial.print(postdata);
    int httpResponseCode = http.POST(postdata);
    if (httpResponseCode > 0) { // Check for the returning code
      String payload = http.getString();
      Serial.println(httpResponseCode); // Print HTTP return code
      Serial.println(payload);          // Print request response payload
    } else {
      Serial.print("Error on sending GET request: ");
      Serial.println(httpResponseCode);
    }
    http.end();

  } else {
    // No parameters were sent or an error occurred
    server.send(400, "text/plain", "Something went wrong contact service");
  }
  updateDis();
}

void sendIp(String ip){
    http.begin("http://192.168.137.1:5000/SendIp");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    postdataIP = "Board=ESP1&ip=" + ip;
    int httpResponseCode = http.POST(postdataIP);
    Serial.println("IpSend Statues: ");
    Serial.println(httpResponseCode);
    http.end();
}

void OpenDoor(){
  Serial.println("Handling fingerprint request...");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  if (server.hasArg("id")) {
    String id = server.arg("id");
    Serial.print("Received id: ");
    Serial.println(id);
    DoorExit(); // Function to open the door
    server.send(200, "text/plain", "Door opened successfully");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
  GetpersonCount();
  updateDis();
}

void updateDis(){
  lcd.clear();
  Serial.println("Enter the password:");
  lcd.setCursor(0, 1);
  lcd.print(" Inside Person : ");
  lcd.print(personCount);
  lcd.setCursor(0, 2);
  lcd.print("Enter the password:");
}


void setup() {
  Serial.begin(9600);
  Displaysetup();
  connectWiFi();
  getTime();
  while (!Serial) {
    ; // Wait for the serial port to connect
  }

  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // Initialize the first fingerprint sensor
  serialPort.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor outer fingerprint!");
  } else {
    Serial.println("Did not find fingerprint sensor outer fingerprint :(");
    while (1) {
      delay(1);
    }
  }
  delay(1000);

  //Door Controll
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT); 
  pinMode(enable1Pin, OUTPUT);
  pinMode(doorsensor, INPUT_PULLUP);
  pinMode(doorsensor1, INPUT_PULLUP);

  pinMode(irSensor, INPUT);
  pinMode(ledPin, OUTPUT);

  ledc_timer_config_t timer_conf;
  timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
  timer_conf.timer_num = LEDC_TIMER_0;
  timer_conf.duty_resolution = LEDC_TIMER_8_BIT;
  timer_conf.freq_hz = freq;
  ledc_timer_config(&timer_conf);

  ledc_channel_config_t ledc_conf;
  ledc_conf.gpio_num = enable1Pin;
  ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_conf.channel = LEDC_CHANNEL_0; 
  ledc_conf.intr_type = LEDC_INTR_DISABLE;
  ledc_conf.timer_sel = LEDC_TIMER_0;
  ledc_conf.duty = 0;
  ledc_channel_config(&ledc_conf);

  if(nowifi){
    Serial.print("Wifi server begin");
    server2.begin();
    offlineDis();
  }else{
    server.on("/enroll", HTTP_POST, handleFingerprint);
    server.on("/doorOpen",HTTP_POST, OpenDoor);
    server.begin();
    IPAddress ip = WiFi.localIP();
    String ipString = ip.toString();
    sendIp(ipString);
    GetpersonCount();
  }

}


void loop() {
  
  if(nowifi){
    getTime();
    Serial.println("Handling Wifi server");
    WiFiClient client2 = server2.available();   // Listen for incoming clients

    if (client2) {                             // If a new client connects,
      Serial.println("New Client.");          // print a message out in the serial port
      String currentLine = "";                // make a String to hold incoming data from the client
      while (client2.connected()) {            // loop while the client's connected
        if (client2.available()) {             // if there's bytes to read from the client,
          char c = client2.read();
          header += c;                        // read a byte, then
          Serial.write(c);                    // print it out the serial monitor
          currentLine += c;
          if (c == '\n') {                    // if the byte is a newline character
            if (currentLine.length() == 2) {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client2.println("HTTP/1.1 200 OK");
              client2.println("Content-type:text/html");
              client2.println("Connection: close");
              client2.println();

              if (header.indexOf("GET /open") >= 0) {             
                client2.println("<p>Door Opening</p>");
                DoorExit();
              }
              if (header.indexOf("GET /connect") >= 0) {             
                client2.println("<p>Connecting</p>");
                ESP.restart();
              }
              client2.println("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>");
              client2.println("<body><h1>StockSync</h1>");
              client2.println("<h2>Open Door</h2>");
              client2.println("<a href=\"/open\"><button>Open Door</button></a>");
              client2.println("<a href=\"/connect\"><button>Connect WiFi</button></a>");
              client2.println("</body></html>");

              // The HTTP response ends with another blank line
              client2.println();
              // Break out of the while loop
              break;
            } else { // if you got a newline, then clear currentLine
              currentLine = "";
            }
          } else if (c != '\r') {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
          }
        }
      }
    header = "";
      // Close the connection
    client2.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
    }
  }else{
    lcd.setCursor(0, 1);
    lcd.print("Connecting");
    if (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
    }
    Serial.println("Welcome to StockSync!");
    lcd.clear();
    lcd.println("Welcome to StockSync!");
    delay(3000);

    while (true) {
      GetpersonCount();
      updateDis();
      passcode = ""; 
      passcodeVerified = false;
      while (!passcodeVerified) {
        server.handleClient();
        getTime();
        if (kpd.getKeys()) {
          for (int i = 0; i < LIST_MAX; i++) {
            if (kpd.key[i].stateChanged && kpd.key[i].kstate == PRESSED) {
              Serial.println(kpd.key[i].kchar);
              
              if (kpd.key[i].kchar == '#') {
                if (passcode == "8504") {
                  Serial.println("Passcode OK!");
                  lcd.clear();
                  lcd.print("Passcode OK!");
                  while (getFingerprintID() == 2);
                  delay(1000);
                  passcodeVerified = true; 
                  break; 
                } else {
                  Serial.println("Invalid Password! Try again.");
                  lcd.clear();
                  lcd.print("Invalid Passcode");
                  passcode = "";
                  delay(5000);
                  updateDis();
                  break;
                }
              } else {
                if (kpd.key[i].kchar == '*') {
                  if (passcode.length() > 0) {
                      String x = passcode.substring(0, passcode.length() - 1);
                      passcode = x;
                      lcd.setCursor(0, 3);
                      lcd.print("");
                      lcd.print(passcode);
                  }
                }else{
                  passcode += kpd.key[i].kchar;
                  lcd.setCursor(0, 3);
                  lcd.print(passcode);
                }

              }
            }
          }
        }
        delay(100); // Add a small delay to avoid busy loop
      }
    }
    }
  
}
