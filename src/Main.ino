#include <Arduino.h>
#include "sistema.h"

// Forward
void setup();
void loop();

void setup() {
  Serial.begin(115200);

  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  lcdPrintDirect("STAT FAX 1904+", "Iniciando...");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Inicializar EEPROM y cargar pacientes (función existente)
  initializeEEPROM();

  // Conectar WiFi (esto puede tardar; se hace en setup como en tu código original)
  connectWiFi();

  Serial.println(" Iniciando STAT FAX 1904+...");
  lcdPrintDirect("STAT FAX 1904+", "Sistema listo");
  delay(1000);

  // Crear colas
  queueKey = xQueueCreate(20, sizeof(char)); // teclas
  queueDisplay = xQueueCreate(10, sizeof(DisplayMessage));
  queueMotor = xQueueCreate(5, sizeof(int));
  queueWiFi = xQueueCreate(5, sizeof(Patient));

  if (!queueKey || !queueDisplay || !queueMotor || !queueWiFi) {
    Serial.println(" Error creando colas FreeRTOS");
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
  }

  // Crear tareas
  xTaskCreatePinnedToCore(taskKeypad, "TaskKeypad", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(taskMenu, "TaskMenu", 8192, NULL, 4, NULL, 1);
  xTaskCreatePinnedToCore(taskLCD, "TaskLCD", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(taskMotor, "TaskMotor", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(taskWiFi, "TaskWiFi", 8192, NULL, 1, NULL, 1);

  // Ejecutar flujo inicial (igual que antes): solicitar contraseña y seleccionar paciente
  // Nota: esas funciones usan waitForKeyFromQueue() internamente (no keypad.waitForKey)
  enterPassword();
  seleccionarPaciente();
  menuPrincipal();

  // loop() queda vacío, todo se procesa por tareas/FSM
}

void loop() {
  // No usar. FreeRTOS maneja tareas.
  vTaskDelay(pdMS_TO_TICKS(1000));
}
