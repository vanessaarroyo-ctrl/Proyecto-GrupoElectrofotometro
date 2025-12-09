#ifndef SISTEMA_H
#define SISTEMA_H

#include <Arduino.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// ---------------- CONFIGURACIÓN LCD ----------------
extern LiquidCrystal_I2C lcd; // definido en sistema.cpp

// ---------------- CONFIGURACIÓN WiFi ----------------
extern const char* ssid;
extern const char* password;
extern const char* serverURL;

// ---------------- PACIENTES ----------------
struct PacienteInfo {
    String id;
    String nombre;
    char tecla;
};
extern PacienteInfo pacientes[];
extern const int numPacientes;

// ---------------- KEYPAD ----------------
extern Keypad keypad;

// ---------------- PINS MOTOR Y SENSOR ----------------
#define OPT_PIN 34
#define IN1 26
#define IN2 25
#define IN3 33
#define IN4 32

// ---------------- EEPROM ----------------
#define EEPROM_SIZE 4096
#define PASSWORD_ADDR 0
#define PATIENT_COUNT_ADDR 10
#define PATIENTS_START_ADDR 100
#define MAX_PATIENTS 50

// ---------------- ESTRUCTURA PACIENTE ----------------
typedef struct {
    int id;
    char sampleType[10];
    char filterName[10];
    float result;
    int timeSeconds;
    long timestamp;
} Patient;

// ---------------- VARIABLES GLOBALES (extern) ----------------
extern int currentMenu;
extern String sampleType;
extern String filterName;
extern int filterPos;
extern int timerSeconds;
extern float lastResult;
extern String currentPatientId;
extern String currentPatientName;
extern String inputPassword;
extern bool authenticated;
extern Patient currentPatient;
extern int patientCount;

// ---------------- COLAS ----------------
typedef struct {
  String line1;
  String line2;
} DisplayMessage;

extern QueueHandle_t queueKey;
extern QueueHandle_t queueDisplay;
extern QueueHandle_t queueMotor;
extern QueueHandle_t queueWiFi;

// ---------------- DECLARACIONES (tus funciones, adaptadas a RTOS) ----------------
void motorStep(int a, int b, int c, int d);
void motorMoveSteps(int steps);
void moveToFilter(int position);
void msg(String a, String b);
void clearSerial();
bool checkPassword();
void enterPassword();
void savePatientToEEPROM(Patient patient, int index);
Patient readPatientFromEEPROM(int index);
void savePatientCount();
int readPatientCount();
void initializeEEPROM();
void seleccionarPaciente();
void saveCurrentPatient();
void showAllPatients();
void showLastPatient();
void enterTime();
void menuPrincipal();
void menuTipoMuestra();
void menuFiltros();
float readOPT();
void temporizador();
void ejecutarPrueba();
void connectWiFi();
void enviarDatosServidor(Patient paciente);

// LCD helpers (use queue or direct)
void lcdPrint(String line1, String line2);
void lcdPrintDirect(String line1, String line2);
void lcdClear();

// Task functions
void taskKeypad(void *pvParameters);
void taskMenu(void *pvParameters);
void taskLCD(void *pvParameters);
void taskMotor(void *pvParameters);
void taskWiFi(void *pvParameters);

// Utility: wait for key from queue (replaces keypad.waitForKey())
char waitForKeyFromQueue();

#endif