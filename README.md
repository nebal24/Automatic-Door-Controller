# Automatic Door Controller

This project is an embedded system built using the **PIC18F4620 microcontroller**.

It simulates an automatic door controller using an ultrasonic sensor, servo motor, LCD, buzzer, ADC inputs, and UART communication.

---

## Overview

The system has two operating modes:

1. **Monitoring Mode**
   - Reads analog input values using ADC
   - Displays voltage and temperature values on the LCD
   - Receives UART characters and prints them on the LCD
   - Allows buzzer control through UART input

2. **Automatic Door Mode**
   - Measures distance using an ultrasonic sensor
   - Opens the door when an object is detected within 15 cm
   - Closes the door when the object moves away
   - Displays door status and distance on the LCD
   - Uses a buzzer alert when the door opens

---

## Main Features

- PIC18F4620-based embedded system
- Ultrasonic distance measurement
- Servo motor door control
- LCD display output
- ADC reading from analog channels
- UART serial communication
- Buzzer feedback
- Mode switching using a push button
- Watchdog timer clearing inside the main loop

---

## Hardware Components

- PIC18F4620 microcontroller
- Ultrasonic sensor
- Servo motor
- 16x2 LCD
- Buzzer
- Push button
- Analog sensors / potentiometers for ADC input
- UART serial interface

---

## Pin Configuration

| Component | PIC Pin |
|---|---|
| Ultrasonic TRIG | RC0 |
| Buzzer | RC1 |
| Servo Motor | RC2 |
| Ultrasonic ECHO | RB2 |
| Mode Button | RB0 |
| UART TX | RC6 |
| UART RX | RC7 |
| LCD Data Bus | PORTD |
| LCD Control Pins | PORTE |

---

## Operating Modes

### Mode 1: ADC and UART Monitoring

In this mode, the system reads two analog channels:

- `AN0` is converted to voltage
- `AN1` is converted to temperature

The LCD displays the voltage and temperature values.

UART input is also supported:
- Normal received characters are printed on the second LCD line
- Sending `B` or `b` toggles the buzzer state

---

### Mode 2: Automatic Door Control

In this mode, the ultrasonic sensor measures the distance of an object.

Behavior:

- If distance is between `1 cm` and `15 cm`:
  - The LCD displays `Door Opened`
  - The servo moves to the open position
  - The buzzer beeps three times

- If no object is detected or the object is farther than `15 cm`:
  - The LCD displays `Door Closed`
  - The servo moves to the closed position
---
## 🛠️ Technologies

- Embedded C  
- PIC18F4620  
- MPLAB X IDE  
- XC8 Compiler  
- ADC  
- UART  
- Timer1  
- LCD interfacing  
- Servo control  
---
## ⚙️ How to Run

1. Open the project in MPLAB X IDE  
2. Select device: PIC18F4620  
3. Build using XC8 compiler  
4. Upload the generated HEX file to the microcontroller or run simulation  
5. Connect components based on the pin configuration

---
## ⚠️ Notes

- Distance threshold is set to 15 cm  
- Timer1 is used for ultrasonic timing  
- ADC is configured for 10-bit readings  
- UART is used for serial communication

## 👥 Contributors

- Nebal  @nebal24
- Afnan  @AfnanSalameh36
---

## Source Files

```text
BasicCircuit.c   # Main application logic and system modes
lcd_x8.c         # LCD driver implementation
lcd_x8.h         # LCD driver header
my_adc.c         # ADC initialization and reading functions
my_adc.h         # ADC header file
my_ser.c         # UART serial communication functions
my_ser.h         # UART header file

