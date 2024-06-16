#ifndef DoorControl_H
#define DoorControl_H

#include <Arduino.h>
#include <driver/ledc.h>  

void Door();

extern int motor1Pin1; 
extern int motor1Pin2; 
extern int enable1Pin; 

extern const int freq;
extern const int pwmChannel;
extern const int resolution;
extern int dutyCycle;

extern const int irSensor;
extern const int ledPin;
extern int irReading;

#endif