// FILE: sistema.h
// -------- QUEUES (defined in main) --------
extern QueueHandle_t queueKey;
extern QueueHandle_t queueDisplay;
extern QueueHandle_t queueMotor;
extern QueueHandle_t queueWiFi;


// -------- FUNCTION PROTOTYPES --------
// LCD helpers
void lcdPrintDirect(const String &l1, const String &l2 = "");
void lcdPrint(const String &l1, const String &l2 = "");
void lcdClear();


// Motor
void motorStep(int a,int b,int c,int d);
void motorMoveSteps(int steps);
void moveToFilter(int position);


// WiFi
void connectWiFiBlocking();
void enviarDatosServidor_enqueue(const Patient &p);


// Password and input
char waitForKeyFromQueue();
void enterPassword();


// EEPROM
void initializeEEPROM();
void savePatientToEEPROM(const Patient &p, int index);
Patient readPatientFromEEPROM(int index);
void savePatientCount();
int readPatientCount();


// Patients/menus
void seleccionarPaciente();
void saveCurrentPatient();
void showAllPatients();
void showLastPatient();
void enterTime();


void menuPrincipal();
void menuTipoMuestra();
void menuFiltros();


// Sensor
float readOPT();
void temporizador();
void ejecutarPrueba_Task(); // non-blocking orchestration callable from menu task


// Tasks (must match prototypes in sistema.cpp)
void taskKeypad(void *pvParameters);
void taskLCD(void *pvParameters);
void taskMotor(void *pvParameters);
void taskWiFi(void *pvParameters);
void taskMenu(void *pvParameters);


#endif // SISTEMA_H