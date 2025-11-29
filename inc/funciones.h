#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// ---------------- STEPPER PINS ----------------
#define IN1 26
#define IN2 25
#define IN3 33
#define IN4 32

#define OPT_PIN 34

extern QueueHandle_t q_keys;
extern QueueHandle_t q_motor;
extern QueueHandle_t q_sensor;

extern int currentMenu;
extern String sampleType;
extern String filterName;
extern int filterPos;
extern float lastResult;
extern int timerSeconds;

// ----- PROTOTIPOS -----
void initMotorPins();
void motorStep(int a, int b, int c, int d);
void motorMoveSteps(int steps);
void moveToFilter(int pos);

void msg(LiquidCrystal_I2C &lcd, String a, String b);
void menuPrincipal(LiquidCrystal_I2C &lcd, int &state);
void menuTipoMuestra(LiquidCrystal_I2C &lcd, int &state);
void menuFiltros(LiquidCrystal_I2C &lcd, int &state);

float readOPT();
void temporizador(LiquidCrystal_I2C &lcd, int sec);
void ejecutarPrueba(LiquidCrystal_I2C &lcd);

void mostrarResultado(LiquidCrystal_I2C &lcd);

// ----- RTOS TASKS -----
void taskKeypad(void *parameter);
void taskLogic(void *parameter);
void taskMotor(void *parameter);
void taskSensor(void *parameter);

#endif
