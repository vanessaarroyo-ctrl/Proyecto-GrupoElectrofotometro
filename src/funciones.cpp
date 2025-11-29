#include "funciones.h"
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

extern LiquidCrystal_I2C lcd;

// ------ STEPPER CONFIG ------
int stepsPerPosition = 1024;
int currentPosition = 0;

// -------- MOTOR -----------
void initMotorPins() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void motorStep(int a, int b, int c, int d) {
  digitalWrite(IN1, a);
  digitalWrite(IN2, b);
  digitalWrite(IN3, c);
  digitalWrite(IN4, d);
  delay(2);
}

void motorMoveSteps(int steps) {
  for (int i = 0; i < steps; i++) {
    motorStep(1,0,0,1);
    motorStep(1,0,0,0);
    motorStep(1,1,0,0);
    motorStep(0,1,0,0);
    motorStep(0,1,1,0);
    motorStep(0,0,1,0);
    motorStep(0,0,1,1);
    motorStep(0,0,0,1);
  }
}

void moveToFilter(int pos) {
  int target = pos % 4;
  int diff = target - currentPosition;
  if (diff < 0) diff += 4;

  motorMoveSteps(diff * stepsPerPosition);
  currentPosition = target;
}

// ------------- LCD MSG -------------
void msg(LiquidCrystal_I2C &lcd, String a, String b) {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(a);
  lcd.setCursor(0,1); lcd.print(b);
  delay(200);
}

// ---------- MENUS ----------
void menuPrincipal(LiquidCrystal_I2C &lcd, int &state) {
  state = 0;
  msg(lcd, " STAT FAX 1904+ ", "1Analizar 2Ver");
}

void menuTipoMuestra(LiquidCrystal_I2C &lcd, int &state) {
  state = 1;
  msg(lcd, "Tipo de muestra", "1P 2S 3O");
}

void menuFiltros(LiquidCrystal_I2C &lcd, int &state) {
  state = 2;
  msg(lcd, "Filtro:", "1R 2V 3B");
}

// ---------- SENSOR OPT101 ----------
float readOPT() {
  const int N = 200;
  long sum = 0;
  for (int i = 0; i < N; i++) sum += analogRead(OPT_PIN);
  return sum / (float)N;
}

// ---------- TEMPORIZADOR ----------
void temporizador(LiquidCrystal_I2C &lcd, int sec) {
  for (int i = sec; i > 0; i--) {
    lcd.clear();
    lcd.setCursor(0,0); lcd.print("Leyendo...");
    lcd.setCursor(0,1); lcd.print("Tiempo: "); lcd.print(i);
    delay(1000);
  }
}

// ---------- PROCESAR PRUEBA ----------
void ejecutarPrueba(LiquidCrystal_I2C &lcd) {
  // mover filtro
  xQueueSend(q_motor, &filterPos, portMAX_DELAY);

  temporizador(lcd, timerSeconds);

  // pedir lectura
  float lectura = 0;
  xQueueSend(q_sensor, &lectura, portMAX_DELAY);

  // esperar respuesta
  xQueueReceive(q_sensor, &lectura, portMAX_DELAY);

  float absorbancia = 1.0 / lectura;
  lastResult = absorbancia;

  lcd.clear();
  lcd.setCursor(0,0); lcd.print("A="); lcd.print(absorbancia, 3);
  lcd.setCursor(0,1); lcd.print(sampleType + "/" + filterName);
  delay(2000);

  int back = 0;
  xQueueSend(q_motor, &back, portMAX_DELAY);

  menuPrincipal(lcd, currentMenu);
}

// ---------- MOSTRAR RESULTADO ----------
void mostrarResultado(LiquidCrystal_I2C &lcd) {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Ultimo valor:");
  lcd.setCursor(0,1); lcd.print(lastResult, 3);
  delay(2000);

  menuPrincipal(lcd, currentMenu);
}


// ======================================================
//                       TAREAS RTOS
// ======================================================

// ---------- LECTURA DEL TECLADO ----------
void taskKeypad(void *parameter) {
  while (true) {
    char key = keypad.getKey();
    if (key) {
      xQueueSend(q_keys, &key, portMAX_DELAY);
    }
    vTaskDelay(10);
  }
}

// ---------- LÓGICA ----------
void taskLogic(void *parameter) {
  char key;

  while (true) {
    if (xQueueReceive(q_keys, &key, portMAX_DELAY)) {

      switch (currentMenu) {

        case 0: // principal
          if (key == '1') menuTipoMuestra(lcd, currentMenu);
          if (key == '2') mostrarResultado(lcd);
          break;

        case 1: // tipo muestra
          if (key == '1') { sampleType = "Pla"; menuFiltros(lcd, currentMenu); }
          if (key == '2') { sampleType = "San"; menuFiltros(lcd, currentMenu); }
          if (key == '3') { sampleType = "Ori"; menuFiltros(lcd, currentMenu); }
          if (key == 'B') menuPrincipal(lcd, currentMenu);
          break;

        case 2: // filtros
          if (key == '1') { filterName = "R"; filterPos = 1; ejecutarPrueba(lcd); }
          if (key == '2') { filterName = "V"; filterPos = 2; ejecutarPrueba(lcd); }
          if (key == '3') { filterName = "B"; filterPos = 3; ejecutarPrueba(lcd); }
          if (key == 'B') menuPrincipal(lcd, currentMenu);
          break;
      }
    }
  }
}

// ---------- MOTOR ----------
void taskMotor(void *parameter) {
  int pos;
  while (true) {
    if (xQueueReceive(q_motor, &pos, portMAX_DELAY)) {
      moveToFilter(pos);
    }
  }
}

// ---------- SENSOR ----------
void taskSensor(void *parameter) {
  float dummy;
  while (true) {
    if (xQueueReceive(q_sensor, &dummy, portMAX_DELAY)) {
      float lectura = readOPT();
      xQueueSend(q_sensor, &lectura, portMAX_DELAY);
    }
  }
}
