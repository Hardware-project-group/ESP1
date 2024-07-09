#include <Arduino.h>
#include <driver/ledc.h>  
#include <HTTPClient.h>
#include <ArduinoJSON.h>
#include "DoorControl.h"
#include "Display.h"
#include "wifi_setup.h"




String serverAddress2;

int doorSensorState;

void offlineDis(){
  lcd.setCursor(0, 2);
  lcd.print("Set wifi connection!");
  lcd.setCursor(5, 1);
  lcd.print("Restric!");
  
}


void getIpOfEsp1(){

    HTTPClient http;
    http.begin("http://192.168.137.1:5000/get-ip");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String postData= "id=ESP3";
    int httpResponseCode = http.POST(postData);
    if (httpResponseCode > 0) {
            String response = http.getString();  // Get the response to the request
            Serial.println(httpResponseCode);    // Print the response code
            Serial.println(response);            // Print the response payload

            // Parse the JSON response
            JsonDocument doc;
            deserializeJson(doc, response);

            String ip_address = doc["ip_address"];
            if (ip_address) {
                Serial.print("IP Address: ");
                Serial.println(ip_address);
                serverAddress2 = "http://" + String(ip_address) + "/capture";
                Serial.println(serverAddress2);
            } else {
                Serial.println("Failed to retrieve IP address.");
            }

        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }

        // Free resources
        http.end();

}
void getIpOfEsp2(){

    HTTPClient http;
    http.begin("http://192.168.137.1:5000/get-ip");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String postData= "id=ESP3";
    int httpResponseCode = http.POST(postData);
    if (httpResponseCode > 0) {
            String response = http.getString();  // Get the response to the request
            Serial.println(httpResponseCode);    // Print the response code
            Serial.println(response);            // Print the response payload

            // Parse the JSON response
            JsonDocument doc;
            deserializeJson(doc, response);

            String ip_address = doc["ip_address"];
            if (ip_address) {
                Serial.print("IP Address: ");
                Serial.println(ip_address);
                serverAddress2 = "http://" + String(ip_address) + "/send";
                Serial.println(serverAddress2);
            } else {
                Serial.println("Failed to retrieve IP address.");
            }

        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }

        // Free resources
        http.end();

}



void Door(int id) {
  
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, dutyCycle); 
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
  lcd.clear();
  lcd.print("Opening the Door....");
  Serial.println("Moving Forward");
  digitalWrite(motor1Pin1, HIGH);  
  digitalWrite(motor1Pin2, LOW); 
  while(true){
    int sensoraRead = digitalRead(doorsensor1);
    if(sensoraRead == LOW){
      digitalWrite(motor1Pin1, LOW);  
      digitalWrite(motor1Pin2, LOW);
      break; 
    }
  }
  lcd.clear();
  lcd.print("Door Open!");
  Serial.println("Motor stopped");
  while(true){
    irReading = digitalRead(irSensor);
    if (irReading == LOW) {
      getIpOfEsp1();
      HTTPClient http;
      http.begin(serverAddress2);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");    
      postData = "ID=" + String(id);
      Serial.println(postData);
      httpCode = http.POST(postData);
      payload = http.getString();
      Serial.print("httpCode: ");
      Serial.println(httpCode); // Print HTTP return code
      Serial.print("payload: ");
      Serial.println(payload);  // Print request response payload
      http.end();
      break;
    }
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for closing");
  lcd.setCursor(0, 1);
  lcd.printf(" Door. Please move ");
  lcd.setCursor(0, 2);
  lcd.printf("away from Door!");
  while(true) {
    irReading = digitalRead(irSensor);
    if (irReading == LOW) {
      digitalWrite(ledPin, HIGH); 
      Serial.println("IR sensor detected. Can't close the door. Moving back!");
      
    } else if (irReading == HIGH) {
      digitalWrite(ledPin, LOW);  
      Serial.println("IR sensor not detected. Closing the door...");
      break; 
    }
  }


  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, dutyCycle);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
  lcd.clear();
  lcd.print("Closing the Door....");
  Serial.println("Moving Backward");
  digitalWrite(motor1Pin1, LOW);  
  digitalWrite(motor1Pin2, HIGH);
  while(true){
    doorSensorState = digitalRead(doorsensor);
    irReading = digitalRead(irSensor);
    if(irReading == LOW){
      digitalWrite(motor1Pin1, LOW); 
      digitalWrite(motor1Pin2, LOW);
      delay(1000);
      digitalWrite(motor1Pin1, HIGH); 
      digitalWrite(motor1Pin2, LOW);
      while(true){
        doorSensorState = digitalRead(doorsensor1);
        if(doorSensorState == LOW){
          digitalWrite(motor1Pin1, LOW); 
          digitalWrite(motor1Pin2, LOW);    
          break;   
        }
      }
      while(true){
        irReading = digitalRead(irSensor);
        if (irReading == LOW) {
          getIpOfEsp1();
          HTTPClient http;
          http.begin(serverAddress2);
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");    
          postData = "ID=" + String(id);
          Serial.println(postData);
          httpCode = http.POST(postData);
          payload = http.getString();
          Serial.print("httpCode: ");
          Serial.println(httpCode); // Print HTTP return code
          Serial.print("payload: ");
          Serial.println(payload);  // Print request response payload
          http.end();
          break;
        }
      }
      digitalWrite(motor1Pin1, LOW);  
      digitalWrite(motor1Pin2, HIGH);
    }
    doorSensorState = digitalRead(doorsensor);
    if(doorSensorState == LOW){
      digitalWrite(motor1Pin1, LOW); 
      digitalWrite(motor1Pin2, LOW);
      doorSensorState = digitalRead(doorsensor1);
      break;
    }
  }
  delay(3000);
  getIpOfEsp2();
  HTTPClient http;
  http.begin(serverAddress2);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    
  postData = "ID=" + String(id);
  Serial.println(postData);
  httpCode = http.POST(postData);
  payload = http.getString();
  Serial.print("httpCode: ");
  Serial.println(httpCode); // Print HTTP return code
  Serial.print("payload: ");
  Serial.println(payload);  // Print request response payload
  http.end();
  if(nowifi){
    offlineDis();
  }
}



void DoorExit() {
  
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, dutyCycle); 
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
  lcd.clear();
  lcd.print("Opening the Door....");
  Serial.println("Moving Forward");
  digitalWrite(motor1Pin1, HIGH);  
  digitalWrite(motor1Pin2, LOW); 
  while(true){
    int sensoraRead = digitalRead(doorsensor1);
    if(sensoraRead == LOW){
      digitalWrite(motor1Pin1, LOW);  
      digitalWrite(motor1Pin2, LOW);
      break; 
    }
  }
  lcd.clear();
  lcd.print("Door Open!");
  Serial.println("Motor stopped");
  while(true){
    irReading = digitalRead(irSensor);
    if (irReading == LOW) {
      break;
    }
  }
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for closing");
  lcd.setCursor(0, 1);
  lcd.printf(" Door. Please move ");
  lcd.setCursor(0, 2);
  lcd.printf("away from Door!");
  while(true) {
    irReading = digitalRead(irSensor);
    if (irReading == LOW) {
      digitalWrite(ledPin, HIGH); 
      Serial.println("IR sensor detected. Can't close the door. Moving back!");
      
    } else if (irReading == HIGH) {
      digitalWrite(ledPin, LOW);  
      Serial.println("IR sensor not detected. Closing the door...");
      break; 
    }
  }


  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, dutyCycle);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
  lcd.clear();
  lcd.print("Closing the Door....");
  Serial.println("Moving Backward");
  digitalWrite(motor1Pin1, LOW);  
  digitalWrite(motor1Pin2, HIGH);
  while(true){
    doorSensorState = digitalRead(doorsensor);
    irReading = digitalRead(irSensor);
    if(irReading == LOW){
      digitalWrite(motor1Pin1, LOW); 
      digitalWrite(motor1Pin2, LOW);
      delay(1000);
      digitalWrite(motor1Pin1, HIGH); 
      digitalWrite(motor1Pin2, LOW);
      while(true){
        doorSensorState = digitalRead(doorsensor1);
        if(doorSensorState == LOW){
          digitalWrite(motor1Pin1, LOW); 
          digitalWrite(motor1Pin2, LOW);    
          break;   
        }
      }
      while(true){
        irReading = digitalRead(irSensor);
        if (irReading == LOW) {
          break;
        }
      }
      digitalWrite(motor1Pin1, LOW);  
      digitalWrite(motor1Pin2, HIGH);
    }
    doorSensorState = digitalRead(doorsensor);
    if(doorSensorState == LOW){
      digitalWrite(motor1Pin1, LOW); 
      digitalWrite(motor1Pin2, LOW);
      doorSensorState = digitalRead(doorsensor1);
      break;
    }
  }
  if(nowifi){
    offlineDis();
  }
}