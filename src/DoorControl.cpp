#include <Arduino.h>
#include <driver/ledc.h>  
#include "DoorControl.h"
#include "Display.h"


void Door() {
  
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, dutyCycle); 
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
  lcd.clear();
  lcd.print("Opening the Door....");
  Serial.println("Moving Forward");
  digitalWrite(motor1Pin1, LOW);  
  digitalWrite(motor1Pin2, HIGH); 
  delay(4000);  
  lcd.clear();
  lcd.print("Door Open!");
  Serial.println("Motor stopped");
  digitalWrite(motor1Pin1, LOW);  
  digitalWrite(motor1Pin2, LOW);
  delay(10000); 

    lcd.clear();
    lcd.printf("Waiting for closing Door. Please move away from Door!");
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
  digitalWrite(motor1Pin1, HIGH);  
  digitalWrite(motor1Pin2, LOW); 
  delay(4000);  
  lcd.clear();
  lcd.print("Door Close!");
  Serial.println("Motor stopped");
  digitalWrite(motor1Pin1, LOW); 
  digitalWrite(motor1Pin2, LOW);
  delay(1000); 
}
