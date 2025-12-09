#include "sistema.h"

// ---------------- CONFIGURACIONES Y VARIABLES ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "CHAVICAS";
const char* password = "sarah1maria2";
const char* serverURL = "http://192.168.100.7/stat_fax/api/sync_esp32.php";

PacienteInfo pacientes[] = {
    {"P001", "Paciente 1", 'A'},
    {"P002", "Paciente 2", 'B'},
    {"P003", "Paciente 3", 'C'},
    {"P004", "Paciente 4", 'D'},
    {"P005", "Paciente 5", '1'},
    {"P006", "Paciente 6", '2'},
    {"P007", "Paciente 7", '3'}
};
const int numPacientes = 7;

// Keypad config
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

// Motor / EEPROM
int stepsPerPosition = 128;
int currentPosition = 0;

// State vars
int currentMenu = 0;
String sampleType = "";
String filterName = "";
int filterPos = 0;
int timerSeconds = 5;
float lastResult = 0;
String currentPatientId = "";
String currentPatientName = "";
String inputPassword = "";
bool authenticated = false;
Patient currentPatient;
int patientCount = 0;

// Queues
QueueHandle_t queueKey = NULL;
QueueHandle_t queueDisplay = NULL;
QueueHandle_t queueMotor = NULL;
QueueHandle_t queueWiFi = NULL;

// ---------------- LCD helpers ----------------
// Direct immediate print (usa lcd directamente) - útil en setup antes de tareas
void lcdPrintDirect(String line1, String line2 = "") {
    lcd.clear();
    delay(10);
    lcd.setCursor(0,0);
    lcd.print(line1.substring(0,16));
    if (line2 != "") {
      lcd.setCursor(0,1);
      lcd.print(line2.substring(0,16));
    }
}

// En cola: manda mensaje al taskLCD
void lcdPrint(String line1, String line2 = "") {
    if (queueDisplay != NULL) {
        DisplayMessage msg;
        msg.line1 = line1;
        msg.line2 = line2;
        // si la cola está llena, eliminamos el mensaje anterior para priorizar el nuevo
        if (xQueueSend(queueDisplay, &msg, (TickType_t)10) != pdTRUE) {
            // intentar eliminar uno y volver a enviar
            DisplayMessage dummy;
            xQueueReceive(queueDisplay, &dummy, 0);
            xQueueSend(queueDisplay, &msg, 10);
        }
    } else {
        lcdPrintDirect(line1, line2);
    }
}

void lcdClear() {
    lcd.clear();
    vTaskDelay(pdMS_TO_TICKS(10));
}

// Scroll (mantengo misma lógica)
void lcdScrollText(String text, int line) {
    lcd.setCursor(0, line);
    lcd.print("                ");
    if (text.length() > 16) {
        for (int i = 0; i < text.length() - 15; i++) {
            lcd.setCursor(0, line);
            lcd.print(text.substring(i, i+16));
            vTaskDelay(pdMS_TO_TICKS(300));
        }
    } else {
        lcd.setCursor(0,line);
        lcd.print(text);
    }
}

// ---------------- MOTOR ----------------
void motorStep(int a, int b, int c, int d) {
    digitalWrite(IN1, a);
    digitalWrite(IN2, b);
    digitalWrite(IN3, c);
    digitalWrite(IN4, d);
    vTaskDelay(pdMS_TO_TICKS(3));
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

void moveToFilter(int position) {
    int target = position % 4;
    int diff = target - currentPosition;
    if (diff < 0) diff += 4;
    motorMoveSteps(diff * stepsPerPosition);
    currentPosition = target;

    Serial.println("Motor movido a posición: " + String(target));
    lcdPrint("Motor posicion:", "Filtro " + String(target));
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// ---------------- Serial helper ----------------
void msg(String a, String b) {
    Serial.println("=== " + a + " ===");
    Serial.println(b);
    Serial.println();
    lcdPrint(a, b);
}

void clearSerial() {
    for (int i = 0; i < 50; i++) {
        Serial.println();
    }
}

// ---------------- WiFi / http ----------------
void connectWiFi() {
    Serial.println();
    Serial.println("=== CONFIGURACIÓN WiFi ===");
    Serial.println("SSID: " + String(ssid));
    lcdPrint("Conectando WiFi", String("SSID: ") + String(ssid));

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        Serial.print(".");

        if (attempts % 3 == 0) {
            String puntos = "";
            for (int i = 0; i < (attempts % 10); i++) puntos += ".";
            lcdPrint("Conectando WiFi", puntos);
        }
        attempts++;
        if (attempts % 5 == 0) {
            Serial.println();
            Serial.print("Intento " + String(attempts) + "/30 - ");
            lcdPrint("Intento " + String(attempts), "de 30");
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(" CONECTADO A WiFi!");
        Serial.print(" IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print(" Señal: ");
        Serial.println(WiFi.RSSI());
        Serial.print(" Gateway: ");
        Serial.println(WiFi.gatewayIP());
        lcdPrint("WiFi Conectado!", "IP: " + WiFi.localIP().toString());
        vTaskDelay(pdMS_TO_TICKS(2000));
        lcdPrint("Señal:", String(WiFi.RSSI()) + " dBm");
    } else {
        Serial.println(" ERROR CONECTANDO A WiFi");
        lcdPrint("Error WiFi", "Modo local");
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void enviarDatosServidor(Patient paciente) {
    // ahora encolaremos y dejaremos que taskWiFi lo envíe (no bloquear)
    if (queueWiFi != NULL) {
        if (xQueueSend(queueWiFi, &paciente, (TickType_t)10) != pdTRUE) {
            // cola ocupada: intentar eliminar el más antiguo y volver a enviar
            Patient dummy;
            xQueueReceive(queueWiFi, &dummy, 0);
            xQueueSend(queueWiFi, &paciente, 10);
        }
    } else {
        // si por alguna razón la cola no existe, enviar directo (comportamiento anterior)
        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            http.begin(serverURL);
            http.addHeader("Content-Type", "application/json");
            String jsonData = "{";
            jsonData += "\"id_paciente\":\"" + currentPatientId + "\",";
            jsonData += "\"nombre_paciente\":\"" + currentPatientName + "\",";
            jsonData += "\"tipo_muestra\":\"" + sampleType + "\",";
            jsonData += "\"filtro\":\"" + filterName + "\",";
            jsonData += "\"resultado\":" + String(paciente.result, 3) + ",";
            jsonData += "\"tiempo_segundos\":" + String(paciente.timeSeconds);
            jsonData += "}";
            Serial.println(" Enviando datos al servidor...");
            lcdPrint("Enviando datos", "al servidor...");
            int httpResponseCode = http.POST(jsonData);
            if (httpResponseCode > 0) {
                Serial.println(" Datos enviados al servidor. Código: " + String(httpResponseCode));
                lcdPrint("Datos enviados", "Cod: " + String(httpResponseCode));
            } else {
                Serial.println(" Error enviando datos: " + String(httpResponseCode));
                lcdPrint("Error envio", "Cod: " + String(httpResponseCode));
            }
            http.end();
        } else {
            Serial.println(" WiFi no disponible - Datos guardados solo localmente");
            lcdPrint("Sin conexion", "Guardado local");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ---------------- PASSWORD ----------------
bool checkPassword() {
    String storedPassword = "1234";
    return inputPassword == storedPassword;
}

// waitForKeyFromQueue() implementa el comportamiento de keypad.waitForKey()
// pero a partir de la cola creada por taskKeypad
char waitForKeyFromQueue() {
    char key = 0;
    if (queueKey == NULL) return 0;
    // Bloquear hasta recibir una tecla
    if (xQueueReceive(queueKey, &key, portMAX_DELAY) == pdTRUE) {
        return key;
    }
    return 0;
}

void enterPassword() {
    clearSerial();
    Serial.println("=== SISTEMA BLOQUEADO ===");
    Serial.println("Ingrese contraseña de 4 dígitos:");
    Serial.println("(Aparecerá como: ****)");
    lcdPrint("Sistema Bloqueado", "Pass: ____");

    inputPassword = "";
    while (inputPassword.length() < 4) {
        char key = waitForKeyFromQueue(); // ahora bloquea por la cola
        if (key >= '0' && key <= '9') {
            inputPassword += key;
            Serial.print("*");
            String asteriscos = "";
            for (int i = 0; i < inputPassword.length(); i++) asteriscos += "*";
            lcdPrint("Ingrese password:", asteriscos);
        }
    }
    Serial.println();

    if (checkPassword()) {
        authenticated = true;
        Serial.println(" Contraseña correcta. Acceso permitido.");
        lcdPrint("Acceso Permitido", "Bienvenido!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    } else {
        Serial.println(" Contraseña incorrecta. Intente nuevamente.");
        lcdPrint("Pass Incorrecta", "Intente de nuevo");
        vTaskDelay(pdMS_TO_TICKS(2000));
        enterPassword(); // repetir
    }
}

// ---------------- EEPROM ----------------
void savePatientToEEPROM(Patient patient, int index) {
    int address = PATIENTS_START_ADDR + (index * sizeof(Patient));
    EEPROM.put(address, patient);
    EEPROM.commit();
}

Patient readPatientFromEEPROM(int index) {
    Patient patient;
    int address = PATIENTS_START_ADDR + (index * sizeof(Patient));
    EEPROM.get(address, patient);
    return patient;
}

void savePatientCount() {
    EEPROM.put(PATIENT_COUNT_ADDR, patientCount);
    EEPROM.commit();
}

int readPatientCount() {
    int count = 0;
    EEPROM.get(PATIENT_COUNT_ADDR, count);
    return count;
}

void initializeEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    patientCount = readPatientCount();
    Serial.println(" Pacientes en memoria: " + String(patientCount));
    lcdPrint("Cargando datos...", "Pac: " + String(patientCount));
}

// ---------------- PACIENTES ----------------
void seleccionarPaciente() {
    clearSerial();
    Serial.println("=== SELECCIÓN DE PACIENTE ===");
    Serial.println("Use las teclas para seleccionar:");
    Serial.println();

    for (int i = 0; i < numPacientes; i++) {
        Serial.println(String(pacientes[i].tecla) + " : " + pacientes[i].id + " - " + pacientes[i].nombre);
    }
    Serial.println();
    Serial.println("Presione la tecla del paciente...");

    lcdPrint("Seleccione", "Paciente:");
    vTaskDelay(pdMS_TO_TICKS(1500));
    lcdPrint("A: P001  B: P002", "C: P003  D: P004");
    vTaskDelay(pdMS_TO_TICKS(2000));
    lcdPrint("1: P005  2: P006", "3: P007");

    bool pacienteSeleccionado = false;
    while (!pacienteSeleccionado) {
        char tecla = waitForKeyFromQueue();
        for (int i = 0; i < numPacientes; i++) {
            if (tecla == pacientes[i].tecla) {
                currentPatientId = pacientes[i].id;
                currentPatientName = pacientes[i].nombre;
                currentPatient.id = i + 1;

                Serial.println();
                Serial.println(" PACIENTE SELECCIONADO:");
                Serial.println("ID: " + currentPatientId);
                Serial.println("Nombre: " + currentPatientName);
                Serial.println("Tecla: " + String(tecla));

                lcdPrint("Paciente:", currentPatientId);
                vTaskDelay(pdMS_TO_TICKS(1000));
                lcdPrint("Nombre:", currentPatientName);
                vTaskDelay(pdMS_TO_TICKS(1000));
                lcdPrint("Seleccionado!", "Tecla: " + String(tecla));

                pacienteSeleccionado = true;
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;
            }
        }

        if (!pacienteSeleccionado) {
            Serial.println("Tecla inválida. Intente nuevamente.");
            lcdPrint("Tecla invalida", "Intente de nuevo");
            vTaskDelay(pdMS_TO_TICKS(1000));
            lcdPrint("Seleccione", "Paciente:");
        }
    }
}

void saveCurrentPatient() {
    if (patientCount < MAX_PATIENTS) {
        sampleType.toCharArray(currentPatient.sampleType, 10);
        filterName.toCharArray(currentPatient.filterName, 10);
        currentPatient.result = lastResult;
        currentPatient.timeSeconds = timerSeconds;
        currentPatient.timestamp = millis();

        savePatientToEEPROM(currentPatient, patientCount);
        patientCount++;
        savePatientCount();

        Serial.println();
        Serial.println("DATOS GUARDADOS:");
        Serial.println("Paciente: " + currentPatientId + " - " + currentPatientName);
        Serial.println("Muestra: " + sampleType);
        Serial.println("Filtro: " + filterName);
        Serial.println("Resultado: " + String(lastResult, 3));
        Serial.println("Tiempo: " + String(timerSeconds) + "s");
        Serial.println("Total en memoria: " + String(patientCount));

        lcdPrint("Datos Guardados", "Pac: " + currentPatientId);
        vTaskDelay(pdMS_TO_TICKS(1000));
        lcdPrint("Res: " + String(lastResult, 3), "Tiempo: " + String(timerSeconds) + "s");
        vTaskDelay(pdMS_TO_TICKS(2000));
        lcdPrint("Total registros:", String(patientCount));

        // Enviar al servidor web (ahora no bloquea)
        enviarDatosServidor(currentPatient);

    } else {
        Serial.println("Error: Memoria llena. No se pueden guardar más pacientes.");
        lcdPrint("Memoria llena", "No mas pacientes");
    }
}

void showAllPatients() {
    clearSerial();
    Serial.println("=== HISTORIAL COMPLETO ===");
    Serial.println("Paciente actual: " + currentPatientId + " - " + currentPatientName);
    Serial.println("Total de registros: " + String(patientCount));
    Serial.println("==========================");

    lcdPrint("Historial Completo", "Reg: " + String(patientCount));
    vTaskDelay(pdMS_TO_TICKS(2000));

    if (patientCount == 0) {
        Serial.println("No hay análisis registrados en la memoria.");
        lcdPrint("No hay registros", "en memoria");
    } else {
        for (int i = 0; i < patientCount; i++) {
            Patient p = readPatientFromEEPROM(i);

            String nombrePaciente = "Desconocido";
            for (int j = 0; j < numPacientes; j++) {
                if (pacientes[j].id == String(p.id)) {
                    nombrePaciente = pacientes[j].nombre;
                    break;
                }
            }

            Serial.println("Análisis #" + String(i+1) + ":");
            Serial.println("   Paciente: " + String(p.id) + " - " + nombrePaciente);
            Serial.println("   Muestra: " + String(p.sampleType));
            Serial.println("   Filtro: " + String(p.filterName));
            Serial.println("   Resultado: " + String(p.result, 3));
            Serial.println("   Tiempo: " + String(p.timeSeconds) + "s");
            Serial.println("   Timestamp: " + String(p.timestamp));
            Serial.println();

            lcdPrint("Registro #" + String(i+1), "Pac: " + String(p.id));
            vTaskDelay(pdMS_TO_TICKS(1500));
            lcdPrint("Muestra: " + String(p.sampleType), "Filtro: " + String(p.filterName));
            vTaskDelay(pdMS_TO_TICKS(1500));
            lcdPrint("Resultado:", String(p.result, 3));
            vTaskDelay(pdMS_TO_TICKS(1500));
        }
    }
    Serial.println("Presione # para volver al menú principal...");
    lcdPrint("Presione #", "para volver");

    while (true) {
        char key = waitForKeyFromQueue();
        if (key == '#') {
            menuPrincipal();
            break;
        }
    }
}

void showLastPatient() {
    clearSerial();
    if (patientCount > 0) {
        Patient lastPatient = readPatientFromEEPROM(patientCount - 1);

        String nombrePaciente = "Desconocido";
        for (int j = 0; j < numPacientes; j++) {
            if (pacientes[j].id == String(lastPatient.id)) {
                nombrePaciente = pacientes[j].nombre;
                break;
            }
        }

        Serial.println("=== ÚLTIMO ANÁLISIS REGISTRADO ===");
        Serial.println("Paciente: " + String(lastPatient.id) + " - " + nombrePaciente);
        Serial.println("Muestra: " + String(lastPatient.sampleType));
        Serial.println("Filtro: " + String(lastPatient.filterName));
        Serial.println("Resultado: " + String(lastPatient.result, 3));
        Serial.println("Tiempo: " + String(lastPatient.timeSeconds) + "s");
        Serial.println("Timestamp: " + String(lastPatient.timestamp));

        lcdPrint("Ultimo Analisis", "Pac: " + String(lastPatient.id));
        vTaskDelay(pdMS_TO_TICKS(1500));
        lcdPrint("Muestra:", String(lastPatient.sampleType));
        vTaskDelay(pdMS_TO_TICKS(1500));
        lcdPrint("Filtro:", String(lastPatient.filterName));
        vTaskDelay(pdMS_TO_TICKS(1500));
        lcdPrint("Resultado:", String(lastPatient.result, 3));
        vTaskDelay(pdMS_TO_TICKS(1500));
    } else {
        Serial.println("No hay análisis registrados.");
        lcdPrint("No hay analisis", "registrados");
    }
    Serial.println();
    Serial.println("Presione # para volver al menú principal...");
    lcdPrint("Presione #", "para volver");

    while (true) {
        char key = waitForKeyFromQueue();
        if (key == '#') {
            menuPrincipal();
            break;
        }
    }
}

// ---------------- TIEMPO ----------------
void enterTime() {
    clearSerial();
    Serial.println("=== CONFIGURACIÓN DE TIEMPO ===");
    Serial.println("Ingrese tiempo en segundos (1-999):");
    Serial.println("Presione # para finalizar");
    Serial.println("Presione * para borrar");

    lcdPrint("Configurar Tiempo", "1-999 segundos");
    vTaskDelay(pdMS_TO_TICKS(1500));
    lcdPrint("Ingrese tiempo:", "#=Fin  *=Borrar");

    String timeInput = "";
    while (true) {
        char key = waitForKeyFromQueue();
        if (key == '#') {
            if (timeInput.length() > 0) {
                timerSeconds = timeInput.toInt();
                if (timerSeconds < 1) timerSeconds = 1;
                if (timerSeconds > 999) timerSeconds = 999;
                break;
            } else {
                Serial.println("Tiempo no puede estar vacío. Ingrese al menos 1 dígito.");
                lcdPrint("Error: tiempo", "vacio. Ingrese");
            }
        } else if (key >= '0' && key <= '9' && timeInput.length() < 3) {
            timeInput += key;
            Serial.print(key);
            lcdPrint("Tiempo:", timeInput + " segundos");
        } else if (key == '*') {
            if (timeInput.length() > 0) {
                timeInput.remove(timeInput.length() - 1);
                Serial.println();
                Serial.println("Tiempo actual: " + timeInput);
                lcdPrint("Tiempo:", timeInput + " segundos");
            }
        }
    }
    Serial.println();
    Serial.println("Tiempo configurado: " + String(timerSeconds) + " segundos");
    lcdPrint("Tiempo configurado:", String(timerSeconds) + " segundos");
    vTaskDelay(pdMS_TO_TICKS(2000));

    Serial.println("Presione # para volver al menú principal...");
    lcdPrint("Presione #", "para volver");

    while (true) {
        char key = waitForKeyFromQueue();
        if (key == '#') {
            menuPrincipal();
            break;
        }
    }
}

// ---------------- MENUS ----------------
void menuPrincipal() {
    currentMenu = 0;
    clearSerial();
    Serial.println("=== STAT FAX 1904+ ===");
    Serial.println("Paciente: " + currentPatientId + " - " + currentPatientName);
    Serial.println("Registros: " + String(patientCount));
    Serial.println("=======================");
    Serial.println("1: Realizar Análisis");
    Serial.println("2: Ver último análisis");
    Serial.println("3: Ver historial completo");
    Serial.println("4: Configurar tiempo");
    Serial.println("B: Cambiar paciente");
    Serial.println();
    Serial.println("Seleccione opción...");

    lcdPrint("STAT FAX 1904+", "Pac: " + currentPatientId);
    vTaskDelay(pdMS_TO_TICKS(1500));
    lcdPrint("1:Analizar 2:Ver", "3:Hist 4:Tmp B:Cam");
}

void menuTipoMuestra() {
    currentMenu = 1;
    clearSerial();
    Serial.println("=== TIPO DE MUESTRA ===");
    Serial.println("Paciente: " + currentPatientId);
    Serial.println("=======================");
    Serial.println("A: Plasma");
    Serial.println("B: Sangre");
    Serial.println("C: Orina");
    Serial.println("D: Volver");
    Serial.println();
    Serial.println("Seleccione tipo de muestra...");

    lcdPrint("Tipo de Muestra", "Pac: " + currentPatientId);
    vTaskDelay(pdMS_TO_TICKS(1500));
    lcdPrint("A:Plasma B:Sangre", "C:Orina  D:Volver");
}

void menuFiltros() {
    currentMenu = 2;
    clearSerial();
    Serial.println("=== SELECCIÓN DE FILTRO ===");
    Serial.println("Paciente: " + currentPatientId);
    Serial.println("Muestra: " + sampleType);
    Serial.println("===========================");
    Serial.println("A: Rojo");
    Serial.println("B: Verde");
    Serial.println("C: Azul");
    Serial.println("D: Volver");
    Serial.println();
    Serial.println("Seleccione filtro...");

    lcdPrint("Seleccion Filtro", "Muestra: " + sampleType);
    vTaskDelay(pdMS_TO_TICKS(1500));
    lcdPrint("A:Rojo  B:Verde", "C:Azul   D:Volver");
}

// ---------------- SENSOR ----------------
float readOPT() {
    const int N = 200;
    long sum = 0;
    for (int i = 0; i < N; i++) {
        sum += analogRead(OPT_PIN);
        if (i % 40 == 0) {
            int progreso = (i * 100) / N;
            lcdPrint("Leyendo sensor...", String(progreso) + "%");
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }

    float lectura = sum / (float)N;
    Serial.println("Lectura OPT: " + String(lectura));
    lcdPrint("Lectura OPT:", String(lectura, 1));
    vTaskDelay(pdMS_TO_TICKS(1000));

    return lectura;
}

// ---------------- TEMPORIZADOR ----------------
void temporizador() {
    currentMenu = 3;
    Serial.println("Iniciando temporizador...");
    lcdPrint("Iniciando", "temporizador...");
    vTaskDelay(pdMS_TO_TICKS(1000));

    for (int i = timerSeconds; i > 0; i--) {
        Serial.println("Tiempo restante: " + String(i) + " segundos");
        if (i <= 10 || i % 5 == 0) {
            lcdPrint("Tiempo restante:", String(i) + " segundos");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    Serial.println("Temporizador finalizado");
    lcdPrint("Temporizador", "finalizado!");
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// ---------------- EJECUTAR PRUEBA ----------------
void ejecutarPrueba() {
    clearSerial();
    Serial.println("=== INICIANDO PRUEBA ===");
    Serial.println("Paciente: " + currentPatientId + " - " + currentPatientName);
    Serial.println("Muestra: " + sampleType);
    Serial.println("Filtro: " + filterName);
    Serial.println("Tiempo: " + String(timerSeconds) + "s");
    Serial.println("=========================");

    lcdPrint("INICIANDO PRUEBA", "Pac: " + currentPatientId);
    vTaskDelay(pdMS_TO_TICKS(1500));
    lcdPrint("Muestra: " + sampleType, "Filtro: " + filterName);
    vTaskDelay(pdMS_TO_TICKS(1500));
    lcdPrint("Tiempo:", String(timerSeconds) + " segundos");
    vTaskDelay(pdMS_TO_TICKS(1500));

    // mover al filtro correcto (enviamos comando al taskMotor para no bloquear)
    if (queueMotor != NULL) {
        int pos = filterPos;
        xQueueSend(queueMotor, &pos, (TickType_t)10);
        // Esperamos a que taskMotor actualice currentPosition (por simplicidad, mantener moveToFilter sin cola)
        // Para preservar lógica original (motor blocking) llamamos moveToFilter directamente:
        moveToFilter(filterPos);
    } else {
        moveToFilter(filterPos);
    }

    temporizador();

    float intensidad = readOPT();

    float absorbancia = 1.0 / intensidad;
    lastResult = absorbancia;

    clearSerial();
    Serial.println("=== RESULTADO ===");
    Serial.println("Paciente: " + currentPatientId + " - " + currentPatientName);
    Serial.println("Muestra: " + sampleType);
    Serial.println("Filtro: " + filterName);
    Serial.println("Absorbancia: " + String(absorbancia, 3));
    Serial.println("Tiempo: " + String(timerSeconds) + "s");
    Serial.println("=================");

    lcdPrint("=== RESULTADO ===", "Pac: " + currentPatientId);
    vTaskDelay(pdMS_TO_TICKS(1500));
    lcdPrint("Muestra: " + sampleType, "Filtro: " + filterName);
    vTaskDelay(pdMS_TO_TICKS(1500));
    lcdPrint("Absorbancia:", String(absorbancia, 3));
    vTaskDelay(pdMS_TO_TICKS(2000));
    lcdPrint("Tiempo: " + String(timerSeconds) + "s", "Guardando...");

    saveCurrentPatient();

    Serial.println("Presione # para volver al menú principal...");
    lcdPrint("Presione # para", "volver al menu");

    while (true) {
        char key = waitForKeyFromQueue();
        if (key == '#') {
            moveToFilter(0);
            menuPrincipal();
            break;
        }
    }
}

// ---------------- TASKS ----------------

// Task que lee el keypad y envia teclas a la cola cada 10ms
void taskKeypad(void *pvParameters) {
    (void) pvParameters;
    for (;;) {
        char key = keypad.getKey();
        if (key) {
            if (queueKey != NULL) {
                xQueueSend(queueKey, &key, (TickType_t)0);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // 10 ms
    }
}

// Task LCD: consume queueDisplay y muestra en LCD. Refresca cada 100ms
void taskLCD(void *pvParameters) {
    (void) pvParameters;
    DisplayMessage msg;
    for (;;) {
        if (queueDisplay != NULL) {
            if (xQueueReceive(queueDisplay, &msg, pdMS_TO_TICKS(100)) == pdTRUE) {
                lcdClear();
                lcd.setCursor(0,0);
                lcd.print(msg.line1.substring(0,16));
                if (msg.line2 != "") {
                    lcd.setCursor(0,1);
                    lcd.print(msg.line2.substring(0,16));
                }
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// Task Motor: consume queueMotor (posiciones). (si no hay comandos permanece inactivo)
void taskMotor(void *pvParameters) {
    (void) pvParameters;
    int pos;
    for (;;) {
        if (queueMotor != NULL) {
            if (xQueueReceive(queueMotor, &pos, portMAX_DELAY) == pdTRUE) {
                moveToFilter(pos); // bloqueará hasta completar (como en tu código original)
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

// Task WiFi: envía pacientes al servidor en background
void taskWiFi(void *pvParameters) {
    (void) pvParameters;
    Patient p;
    for (;;) {
        if (queueWiFi != NULL) {
            if (xQueueReceive(queueWiFi, &p, portMAX_DELAY) == pdTRUE) {
                // enviar
                if (WiFi.status() == WL_CONNECTED) {
                    HTTPClient http;
                    http.begin(serverURL);
                    http.addHeader("Content-Type", "application/json");
                    String jsonData = "{";
                    jsonData += "\"id_paciente\":\"" + currentPatientId + "\",";
                    jsonData += "\"nombre_paciente\":\"" + currentPatientName + "\",";
                    jsonData += "\"tipo_muestra\":\"" + sampleType + "\",";
                    jsonData += "\"filtro\":\"" + filterName + "\",";
                    jsonData += "\"resultado\":" + String(p.result, 3) + ",";
                    jsonData += "\"tiempo_segundos\":" + String(p.timeSeconds);
                    jsonData += "}";
                    Serial.println("(WiFiTask) Enviando datos al servidor...");
                    int code = http.POST(jsonData);
                    if (code > 0) {
                        Serial.println("(WiFiTask) Enviado. Cod: " + String(code));
                        lcdPrint("Datos enviados", "Cod: " + String(code));
                    } else {
                        Serial.println("(WiFiTask) Error envio: " + String(code));
                        lcdPrint("Error envio", "Cod: " + String(code));
                    }
                    http.end();
                } else {
                    Serial.println(" (WiFiTask) Sin WiFi - guardado local");
                    lcdPrint("Sin conexion", "Guardado local");
                }
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

// Task Menu (FSM) central: procesa teclas desde queueKey y llama a tus funciones
void taskMenu(void *pvParameters) {
    (void) pvParameters;
    char key;
    for (;;) {
        if (queueKey != NULL) {
            if (xQueueReceive(queueKey, &key, portMAX_DELAY) == pdTRUE) {
                switch (currentMenu) {
                    case 0: // MENU PRINCIPAL
                        if (key == '1') menuTipoMuestra();
                        else if (key == '2') showLastPatient();
                        else if (key == '3') showAllPatients();
                        else if (key == '4') enterTime();
                        else if (key == 'B') seleccionarPaciente();
                        break;
                    case 1: // TIPO MUESTRA
                        if (key == 'A') { sampleType = "Plasma"; menuFiltros(); }
                        else if (key == 'B') { sampleType = "Sangre"; menuFiltros(); }
                        else if (key == 'C') { sampleType = "Orina"; menuFiltros(); }
                        else if (key == 'D') menuPrincipal();
                        break;
                    case 2: // FILTROS
                        if (key == 'A') { filterName = "Rojo"; filterPos = 1; ejecutarPrueba(); }
                        else if (key == 'B') { filterName = "Verde"; filterPos = 2; ejecutarPrueba(); }
                        else if (key == 'C') { filterName = "Azul"; filterPos = 3; ejecutarPrueba(); }
                        else if (key == 'D') menuPrincipal();
                        break;
                    default:
                        menuPrincipal();
                        break;
                }
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}