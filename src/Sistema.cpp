// FILE: sistema.cpp
lcdPrint("Absorbancia:", String(absorbancia,4)); vTaskDelay(pdMS_TO_TICKS(800));


// save
sampleType.toCharArray(currentPatient.sampleType,10);
filterName.toCharArray(currentPatient.filterName,10);
currentPatient.result = lastResult;
currentPatient.timeSeconds = timerSeconds;
currentPatient.timestamp = millis();
if(patientCount < MAX_PATIENTS){ savePatientToEEPROM(currentPatient, patientCount); patientCount++; savePatientCount(); enviarDatosServidor_enqueue(currentPatient); }


lcdPrint("Prueba finalizada","Presione #");
}


// -------- TASKS IMPLEMENTATION --------
void taskKeypad(void *pvParameters){ (void)pvParameters; for(;;){ char k = keypad.getKey(); if(k){ if(queueKey!=NULL) xQueueSend(queueKey, &k, 0); } vTaskDelay(pdMS_TO_TICKS(10)); } }


void taskLCD(void *pvParameters){ (void)pvParameters; DisplayMessage msg; for(;;){ if(queueDisplay!=NULL){ if(xQueueReceive(queueDisplay, &msg, pdMS_TO_TICKS(200)) == pdTRUE){ lcd.clear(); lcd.setCursor(0,0); lcd.print(msg.line1.substring(0,16)); lcd.setCursor(0,1); lcd.print(msg.line2.substring(0,16)); } } else vTaskDelay(pdMS_TO_TICKS(200)); } }


void taskMotor(void *pvParameters){ (void)pvParameters; int pos; for(;;){ if(queueMotor!=NULL){ if(xQueueReceive(queueMotor, &pos, portMAX_DELAY) == pdTRUE){ moveToFilter(pos); } } else vTaskDelay(pdMS_TO_TICKS(1000)); } }


void taskWiFi(void *pvParameters){ (void)pvParameters; Patient p; connectWiFiBlocking(); for(;;){ if(queueWiFi!=NULL){ if(xQueueReceive(queueWiFi, &p, portMAX_DELAY) == pdTRUE){ if(WiFi.status()==WL_CONNECTED){ HTTPClient http; http.begin(serverURL); http.addHeader("Content-Type","application/json"); String json = "{"; json += "\"id_paciente\":\"" + currentPatientId + "\","; json += "\"nombre_paciente\":\"" + currentPatientName + "\","; json += "\"tipo_muestra\":\"" + String(p.sampleType) + "\","; json += "\"filtro\":\"" + String(p.filterName) + "\","; json += "\"resultado\":" + String(p.result,4) + ","; json += "\"tiempo_segundos\":" + String(p.timeSeconds); json += "}"; int code = http.POST(json); if(code>0) lcdPrint("Datos enviados","Cod: " + String(code)); else lcdPrint("Error envio","Cod: " + String(code)); http.end(); } else { savePatientToEEPROM(p, patientCount); patientCount++; savePatientCount(); lcdPrint("Sin WiFi","Guardado local"); } vTaskDelay(pdMS_TO_TICKS(500)); } } else vTaskDelay(pdMS_TO_TICKS(1000)); } }


void taskMenu(void *pvParameters){ (void)pvParameters; char key; menuPrincipal(); for(;;){ if(queueKey!=NULL){ if(xQueueReceive(queueKey, &key, portMAX_DELAY) == pdTRUE){ switch(currentMenu){ case 0: if(key=='1') menuTipoMuestra(); else if(key=='2') showLastPatient(); else if(key=='3') showAllPatients(); else if(key=='4') enterTime(); else if(key=='B') seleccionarPaciente(); break; case 1: if(key=='A'){ sampleType = "Plasma"; menuFiltros(); } else if(key=='B'){ sampleType = "Sangre"; menuFiltros(); } else if(key=='C'){ sampleType = "Orina"; menuFiltros(); } else if(key=='D') menuPrincipal(); break; case 2: if(key=='A'){ filterName = "Rojo"; filterPos=1; ejecutarPrueba_Task(); } else if(key=='B'){ filterName = "Verde"; filterPos=2; ejecutarPrueba_Task(); } else if(key=='C'){ filterName = "Azul"; filterPos=3; ejecutarPrueba_Task(); } else if(key=='D') menuPrincipal(); break; default: menuPrincipal(); break; } } } else vTaskDelay(pdMS_TO_TICKS(50)); } }

