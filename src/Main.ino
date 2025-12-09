#include <Arduino.h>
#include "sistema.h"


// Queues defined here and referenced in sistema.cpp
QueueHandle_t queueKey = NULL;
QueueHandle_t queueDisplay = NULL;
QueueHandle_t queueMotor = NULL;
QueueHandle_t queueWiFi = NULL;


void setup(){
Serial.begin(115200);
lcd.init(); lcd.backlight();


pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);


initializeEEPROM();


// create queues
queueKey = xQueueCreate(20, sizeof(char));
queueDisplay = xQueueCreate(10, sizeof(DisplayMessage));
queueMotor = xQueueCreate(5, sizeof(int));
queueWiFi = xQueueCreate(5, sizeof(Patient));


if(!queueKey || !queueDisplay || !queueMotor || !queueWiFi){ Serial.println("Error creando colas"); while(1) vTaskDelay(pdMS_TO_TICKS(1000)); }


// create tasks
xTaskCreatePinnedToCore(taskKeypad, "TaskKeypad", 4096, NULL, 3, NULL, 1);
xTaskCreatePinnedToCore(taskMenu, "TaskMenu", 8192, NULL, 4, NULL, 1);
xTaskCreatePinnedToCore(taskLCD, "TaskLCD", 4096, NULL, 2, NULL, 1);
xTaskCreatePinnedToCore(taskMotor, "TaskMotor", 8192, NULL, 2, NULL, 1);
xTaskCreatePinnedToCore(taskWiFi, "TaskWiFi", 8192, NULL, 1, NULL, 1);


// preset default patient
for(int i=0;i<numPacientes;i++){ if(pacientes[i].id=="P001"){ currentPatientId = pacientes[i].id; currentPatientName = pacientes[i].nombre; currentPatient.id = i+1; break; } }


lcdPrint("Sistema listo","Menu principal");
}


void loop(){ vTaskDelay(pdMS_TO_TICKS(1000)); }