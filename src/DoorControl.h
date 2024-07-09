#ifndef DoorControl_H
#define DoorControl_H

#include <Arduino.h>
#include <driver/ledc.h>  


void Door(int id);
void DoorExit();
void doorstate();
void getIpOfEsp1();
void offlineDis();

extern int motor1Pin1; 
extern int motor1Pin2; 
extern int enable1Pin;
extern int doorsensor;
extern int doorsensor1;

extern const int freq;
extern const int pwmChannel;
extern const int resolution;
extern int dutyCycle;

extern const int irSensor;
extern const int ledPin;
extern int irReading;


 // Update with your server address
extern String postData;
extern String payload;
extern int httpCode;



#endif