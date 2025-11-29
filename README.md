# PROTOTIPO DE ESPECTROFOTOMETRO  SPV MEDICAL

Proyecto desarrollado para la materia **IMT 222 SISTEMAS EMBEBIDOS I*, orientado al diseño y validación de un **espectrofotometro de bajo costo**, capaz de medir absorbancia utilizando **tres filtros opticos (rojo, verde y blanco)** y un sistema de control basado en **ESP32 + FreeRTOS**.

Este equipo busca ser una alternativa **accesible, confiable y didactica**, especialmente util para **laboratorios universitarios, practicas estudiantiles e investigacion basica**.


## Descripción del Proyecto

La espectrofotometría es una técnica fundamental utilizada para determinar la concentración de sustancias mediante la medición de la luz absorbida por una muestra. Sin embargo, los equipos comerciales suelen ser costosos y poco accesibles.

Este proyecto propone el desarrollo de un **espectrofotómetro tipo Stat Fax**, utilizando:
- Componentes electrónicos de bajo costo,
- Diseño mecánico 3D,
- Filtros ópticos seleccionados,
- Sensado mediante **OPT101**,  
- Control de filtros mediante motor paso a paso,
- Interfaz LCD + teclado matricial,
- Procesamiento con ESP32.

# Arquitectura del Sistema 


# **Diagrama FSM (para guardar en `/img/fsm.png`)**



# Descripción del Proyecto

La espectrofotometría es una técnica utilizada para determinar la concentración de sustancias mediante la medición de la luz absorbida por una muestra. Sin embargo, los equipos comerciales suelen ser costosos y poco accesibles para laboratorios pequeños o instituciones educativas.

Este proyecto implementa un **espectrofotómetro tipo Stat Fax**, diseñado para ser:

- Económico
- Preciso (±1–2% con OPT101)
- Amigable para estudiantes
- Fácil de operar
- Basado en hardware accesible (ESP32)

Incluye control automático de filtros, interfaz intuitiva y funcionalidades que simulan un espectrofotómetro clínico real.

---

# **Tecnologías Utilizadas**

### **Hardware**
- ✔ ESP32 WROOM
- ✔ OPT101 (sensor óptico)
- ✔ Motor paso a paso + driver ULN2003
- ✔ LEDs de alta intensidad (rojo, verde, blanco)
- ✔ LCD 16x2 (I2C)
- ✔ Teclado matricial 4x4
- ✔ Estructura 3D tipo Stat Fax

### **Software**
- ✔ FreeRTOS (tareas, colas, concurrencia)
- ✔ C/C++ (Arduino Framework)
- ✔ Arduino IDE / PlatformIO
- ✔ SolidWorks (diseño mecánico)
- ✔ Git y GitHub

---

#  **Integrantes y Roles**

| **Vanessa Arroyo Fernández** | Desarrollo del sistema, diseño funcional, documentación |
| **Sarah Chávez Valencia** | Desarrollo del sistema, diseño funcional, documentación |

---

#  **Estado Actual del Proyecto**

### COMPLETADO
- Planteamiento del problema  
- Objetivos general y específicos  
- Marco teórico y análisis del equipo Stat Fax  
- Selección de componentes electrónicos  
- Diseño 3D en SolidWorks    
- Implementación de 4 tareas FreeRTOS:
  - `taskKeypad`
  - `taskLogic`
  - `taskMotor`
  - `taskSensor`
- Comunicación mediante colas RTOS:
  - q_keys
  - q_motor
  - q_sensor  
- Movimiento funcional del motor paso a paso  
- Cálculo básico de absorbancia

### EN PROGRESO
- Optimización del cálculo de absorbancia  
- Validación experimental con cubetas reales  
- Ajuste de tiempos y sensibilidad  
- Integración final con gabinete diseñado en 3D 
- Lectura del OPT101 con promedio de 10 muestras 
- Implementación del FSM en proces

### PENDIENTE
- Validación con curva de Beer–Lambert  
- Pruebas clínicas simuladas  
- Implementación de envío automático de resultados  
- Documentación final del proceso de calibración  


# **Repositorio — Estructura Propuesta**

├── src/
│ ├── funciones.cpp
│ ├── main.ino
│ └── tareas FreeRTOS
├── inc/
│ └── funciones.h
├── img/
│ ├── arquitectura_espectrofotometro.png
│ ├── fsm.png
│ └── fotos_prototipo/
├── docs/
│ ├── Informe.pdf
│ ├── Presentación.pdf
│ └── Diagramas teóricos
└── README.md


---

# Conclusión

Este proyecto constituye una herramienta robusta y accesible para instituciones educativas y laboratorios básicos, integrando principios de espectrofotometría, electrónica biomédica, diseño mecánico y programación embebida con FreeRTOS.

---

