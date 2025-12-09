# PROTOTIPO DE ESPECTROFOTOMETRO  SPV MEDICAL

Proyecto desarrollado para la materia **IMT 222 SISTEMAS EMBEBIDOS I*, orientado al diseño y validación de un **espectrofotometro de bajo costo**, capaz de medir absorbancia utilizando **tres filtros opticos (rojo, verde, blanco y transparente)** y un sistema de control basado en **ESP32 + FreeRTOS**.

Este equipo busca ser una alternativa **accesible, confiable y didactica**, especialmente util para **laboratorios universitarios, practicas estudiantiles e investigacion basica**.

<h2 align="center">Espectrofotómetro en Solid Works</h2>

<p align="center">
  <img src="https://raw.githubusercontent.com/vanessaarroyo-ctrl/Proyecto-GrupoElectrofotometro/main/img/espectrofotometro.png" 
  width="700">
</p>


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

<h2 align="center">Características del Espectrofotómetro</h2>

<p align="center">
  <img src="https://raw.githubusercontent.com/vanessaarroyo-ctrl/Proyecto-GrupoElectrofotometro/main/img/producto.png" width="700">
</p>


# Arquitectura del Sistema 

<h2 align="center">Arquitectura del Sistema</h2>

<p align="center">
  <img src="https://raw.githubusercontent.com/vanessaarroyo-ctrl/Proyecto-GrupoElectrofotometro/main/img/ARQUITECTURA.jpg" width="700">
</p>


# **Diagrama FSM**

<h2 align="center">Diagrama FSM</h2>

<p align="center">
  <img src="https://raw.githubusercontent.com/vanessaarroyo-ctrl/Proyecto-GrupoElectrofotometro/main/img/FSM.png" 
  width="700">
</p>

# Descripción del Proyecto

La espectrofotometría es una técnica utilizada para determinar la concentración de sustancias mediante la medición de la luz absorbida por una muestra. Sin embargo, los equipos comerciales suelen ser costosos y poco accesibles para laboratorios pequeños o instituciones educativas.

Este proyecto implementa un **espectrofotómetro tipo Stat Fax**, diseñado para ser:

- Económico
- Preciso (±1–2% con OPT101)
- Optimizar trazabilidad
- Amigable para el usuario
- Fácil de operar
- Basado en hardware accesible (ESP32)
- Digitalización de resultados

Incluye control automático de filtros, interfaz intuitiva y funcionalidades que simulan un espectrofotómetro clínico real.

---

# **Tecnologías Utilizadas**

### **Hardware**
- ✔ ESP32 WROOM
- ✔ OPT101 (sensor óptico)
- ✔ Motor paso a paso + driver ULN2003
- ✔ LED de alta intensidad
- ✔ LCD 16x2 (I2C)
- ✔ Teclado matricial 4x4
- ✔ Estructura 3D tipo Stat Fax

### **Software**
- ✔ FreeRTOS (tareas, colas, concurrencia)
- ✔ C/C++ (Arduino Framework)
- ✔ Visual Studio (PHP)
- ✔ Arduino IDE / PlatformIO
- ✔ SolidWorks (diseño mecánico)
- ✔ Git y GitHub

---

#  **Parte Electrónica**
Se empleó el software KiCad para realizar el enrutado correspondiente, permitiendo definir las pistas y conexiones necesarias para el desarrollo de la placa PCB.

<h2 align="center">Conexiones del circuito</h2>

<p align="center">
  <img src="https://raw.githubusercontent.com/vanessaarroyo-ctrl/Proyecto-GrupoElectrofotometro/main/img/Conexiones del circuito.jpeg" width="500">
</p>

<h2 align="center">Circuito en formato 3D</h2>

<p align="center">
  <img src="https://raw.githubusercontent.com/vanessaarroyo-ctrl/Proyecto-GrupoElectrofotometro/main/img/Circuito en formato 3D.jpeg" width="500">
</p>

<h2 align="center">Enrutamiento de la PLACA PCB</h2>

<p align="center">
  <img src="https://raw.githubusercontent.com/vanessaarroyo-ctrl/Proyecto-GrupoElectrofotometro/main/img/Placa PCB.jpeg" width="500">
</p>

#  **Integrantes y Roles**

|     **INTEGRANTES**          |                    **ROLES**                            |
|------------------------------|---------------------------------------------------------|
| **Vanessa Arroyo Fernández** | Desarrollo del sistema, diseño funcional, documentación |
| **Sarah Chávez Valencia**    | Desarrollo del sistema, diseño funcional, documentación |

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
- Optimización del cálculo de absorbancia 
- Validación experimental con cubetas reales  
- Ajuste de tiempos y sensibilidad 
- Implementación del FSM
- Validación con curva de Beer–Lambert  
- Pruebas clínicas simuladas
- Implementación de envío automático de resultados  
- Documentación final del proceso de calibración  

### PENDIENTE
- Integración final con gabinete diseñado en 3D 


# **Repositorio — Estructura Propuesta**

<h2 align="center">Repositorio actual</h2>

<p align="center">
  <img src="https://raw.githubusercontent.com/vanessaarroyo-ctrl/Proyecto-GrupoElectrofotometro/main/img/estructurarepositorio.png" width="300">
</p>



---

# Conclusión

Este proyecto constituye un equipo ideal y accesible para instituciones educativas y laboratorios básicos, integrando principios de espectrofotometría, electrónica biomédica, diseño mecánico y programación embebida con FreeRTOS.

---

