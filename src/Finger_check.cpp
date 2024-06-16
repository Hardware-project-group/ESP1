#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <HTTPClient.h>

#include "DoorControl.h"
#include "Finger_check.h"
#include "wifi_setup.h"
#include "Display.h"

const char* serverAddress = "http://192.168.137.1/TestEsp/sendaccess.php";
String postData = "id=";
String payload = "";
int httpCode; 

uint8_t getFingerprintID()
{
    Displaysetup();

    uint8_t p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
        lcd.clear();
        Serial.print("Image taken");
        lcd.print("Image taken");
        delay(1000);
        break;
    case FINGERPRINT_NOFINGER:
        Serial.print("No finger detected");
        lcd.clear();
        lcd.print("No finger detected");
        delay(1000);
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.print("Communication error");
        lcd.clear();
        lcd.print("Communication error. Try again!");
        delay(1000);
        return p;
    case FINGERPRINT_IMAGEFAIL:
        Serial.print("Imaging error");
        lcd.clear();
        lcd.print("Imaging error..Try Again!");
        delay(1000);
        return p;
    default:
        Serial.print("Unknown error");
        lcd.clear();
        lcd.print("Unknown error.Try again!");
        delay(1000);
        return p;
    }

    // OK success!

    p = finger.image2Tz();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.print("Image too messy");
        lcd.clear();
        lcd.print("Image too messy . Try again!");
        delay(1000);
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.print("Communication error");
        lcd.clear();
        lcd.print("Communication error. Try again!");
        delay(1000);
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.print("Could not find fingerprint features");
        lcd.clear();
        lcd.print("Something went wrong!");
        delay(1000);
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.print("Could not find fingerprint features");
        lcd.clear();
        lcd.print("Something went wrong!");
        delay(1000);
        return p;
    default:
        Serial.print("Unknown error");
        delay(1000);
        return p;
    }

    // OK converted!
    p = finger.fingerSearch();
    if (p == FINGERPRINT_OK)
    {
        Serial.print("Fingerprint Confirm");
        delay(100);
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.print("Communication error");
        delay(1000);
        return p;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        Serial.print("Did not find a match");
        lcd.clear();
        lcd.print("Did not find a match");
        delay(5000);
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    // found a match!
    lcd.clear();
    Serial.print("Found ID #");
    lcd.print("Found ID #");
    Serial.print(finger.fingerID);
    lcd.print(finger.fingerID);
    Door();
    
    HTTPClient http;
    http.begin(serverAddress);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    postData = postData + String(finger.fingerID);
     Serial.println(postData);
    httpCode = http.POST(postData);
    payload = http.getString(); 
    Serial.print("httpCode : ");
    Serial.println(httpCode); //--> Print HTTP return code
    Serial.print("payload  : ");
    Serial.println(payload);  //--> Print request response payload
    http.end();
    postData = "id=";
    delay(5000);
    Serial.println(" with confidence of ");
    Serial.println(finger.confidence);
    return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez()
{
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK)
        return -1;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK)
        return -1;

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK)
        return -1;

    // found a match!
    lcd.clear();
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    lcd.print("Found ID #");
    lcd.print(finger.fingerID);
    Door();
    
    HTTPClient http;
    http.begin(serverAddress);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    postData = postData + String(finger.fingerID);
    Serial.println(postData);
    httpCode = http.POST(postData);
    payload = http.getString(); 
    Serial.print("httpCode : ");
    Serial.println(httpCode); //--> Print HTTP return code
    Serial.print("payload  : ");
    Serial.println(payload);  //--> Print request response payload
    http.end();
    postData = "id=";
    delay(5000);
    Serial.println(" with confidence of ");
    Serial.print(finger.confidence);
    return finger.fingerID;
}