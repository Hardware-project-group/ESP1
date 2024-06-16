//Global Section
#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <WiFi.h>
#include <Keypad.h>
#include <HTTPClient.h>
#include <WebServer.h>

#include "Finger_check.h"
#include "enroll.h"
#include "wifi_setup.h"
#include "keyCode.h"
#include "Display.h"
#include "DoorControl.h"

HardwareSerial serialPort(2); 

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&serialPort);

HTTPClient http;
WebServer server(80);

uint8_t id;
uint8_t fingerId;
bool passcodeVerified = false;
String passcode = "";
String postdata = "id=";

//Door Open System
int motor1Pin1 = 15; 
int motor1Pin2 = 2; 
int enable1Pin = 4; 

const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;

const int irSensor = 35;
const int ledPin = 32;
int irReading;
//End

void handleFingerprint() {
  Serial.println("Handling fingerprint request...");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  http.begin("http://10.13.127.54/TestEsp/getdata.php");
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
  lcd.clear();
  lcd.print("Enter the passcode");
}




void setup() {
  Serial.begin(9600);
  Displaysetup();
  connectWiFi();

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


  server.on("/enroll", HTTP_POST, handleFingerprint);
  server.begin();
  Serial.println(WiFi.localIP());

}



void loop() {
  lcd.clear();
  lcd.print("Connecting");
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  Serial.println("Welcome to StockSync!");
  lcd.clear();
  lcd.println("Welcome to StockSync!");
  delay(3000);

  while (true) {
    Serial.println("Enter the password:");
    lcd.clear();
    lcd.print("Enter the password:");
    passcode = ""; // Clear passcode
    passcodeVerified = false; // Reset passcode verification flag
    while (!passcodeVerified) {
      server.handleClient();
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
                passcodeVerified = true; // Set flag to true to exit passcode entry loop
                break; // Exit passcode entry loop
              } else {
                Serial.println("Invalid Password! Try again.");
                lcd.clear();
                lcd.print("Invalid Passcode");
                passcode = "";
                break;
              }
            } else {
              passcode += kpd.key[i].kchar;
              lcd.clear();
              lcd.print(passcode);
            }
          }
        }
      }
      delay(100); // Add a small delay to avoid busy loop
    }
  }
}
