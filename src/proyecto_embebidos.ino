#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include "funciones.h"

// ----- LCD -----
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ----- KEYPAD -----
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {19, 18, 5, 17};
byte colPins[COLS] = {16, 4, 0, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


// --------- VARIABLES GLOBALES ----------
int currentMenu = 0;
String sampleType = "";
String filterName = "";
int filterPos = 0;

float lastResult = 0;
int timerSeconds = 5;

// --------- RTOS HANDLES ----------
TaskHandle_t t_keypad;
TaskHandle_t t_logic;
TaskHandle_t t_motor;
TaskHandle_t t_sensor;


// ---------- QUEUE (comunicación entre tareas) ----------
QueueHandle_t q_keys;
QueueHandle_t q_motor;
QueueHandle_t q_sensor;


// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  // Init stepper pins
  initMotorPins();

  // --- Queues ---
  q_keys   = xQueueCreate(10, sizeof(char));
  q_motor  = xQueueCreate(5,  sizeof(int));
  q_sensor = xQueueCreate(5,  sizeof(float));

  msg(lcd, "Iniciando...", "");
  delay(500);

  menuPrincipal(lcd, currentMenu);

  // ----- CREATE TASKS -----
  xTaskCreatePinnedToCore(taskKeypad,  "KEYPAD", 4096, NULL, 1, &t_keypad,  1);
  xTaskCreatePinnedToCore(taskLogic,   "LOGIC",  4096, NULL, 2, &t_logic,   1);
  xTaskCreatePinnedToCore(taskMotor,   "MOTOR",  4096, NULL, 2, &t_motor,   1);
  xTaskCreatePinnedToCore(taskSensor,  "SENSOR", 4096, NULL, 2, &t_sensor,  0);
}

void loop() {
  // FreeRTOS se encarga — loop queda vacío
}
