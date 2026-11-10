Automated Nursing Alert System

A nurse call system for ten wards, built around a PIC16F877A microcontroller and simulated in Proteus. When a patient presses a ward call switch, the system stores the request, sounds a short buzzer alert, lights a red LED, and shows the highest-priority pending ward on a seven-segment display. A reset switch lets the nurse clear the displayed call.

Features

10 ward call inputs (W0 to W9) plus a nurse reset switch

Call storage: each call is held in a pending-call bit mask, so it stays active after the patient releases the switch and no request is lost when several wards call at once

Priority display: the highest-numbered pending ward (9 down to 0) is shown on the seven-segment display

Audible alert: a short buzzer pulse (about 0.4 s) on each new call, not a continuous alarm, to keep the ward quiet

Visual alert: the red LED stays on while any call is pending

Reset: clears the displayed ward and shows the next pending one

Interrupt-driven: Timer1 interrupt scans the switches and times the buzzer, with no blocking delays

Pin configuration

Ward 0 to 7 switches	RB0 to RB7

Ward 8 switch	RA0

Ward 9 switch	RA1

Reset switch	RA2

Seven-segment display	PORTD (RD0 to RD6)

Red LED	RC0

Buzzer	RC1

Switches use 10 kΩ pull-down resistors (a pressed switch reads HIGH). Segments and LED use 220 Ω current-limiting resistors. A real buzzer should be driven through an NPN transistor.

How it works

Timer1 overflows about every 65 ms (8 MHz clock, 1:2 prescaler) and triggers the interrupt.
In each interrupt, the ward switches are read and any pressed ward is added to pending_wards.
If a new call appears (compared with the previous state), the buzzer timer starts.
A press on the reset switch clears the ward currently shown.
The program picks the highest pending ward, then updates the display, LED and buzzer.

Repository structure

PIC_Code/
  Nurse_Alert_System.c     Source code (mikroC PRO for PIC)
  MyProject.hex            Compiled program for the simulation

Proteus/
  Nurse_Alert_System.pdsprj   Proteus simulation

How to run the simulation

Download or clone this repository and keep the folder structure unchanged.
Open Proteus/Nurse_Alert_System.pdsprj in Proteus 8.
The PIC loads PIC_Code/MyProject.hex (the path is relative). The clock is set to 8 MHz.
Press Play and click the ward switches. The display, LED and buzzer respond, and the reset switch clears calls.

Limitations and future improvements

A call stays pending only in RAM, so it is lost on power failure. Possible fix: log calls to EEPROM.
Priority is fixed by ward number, not by how urgent the call is.
Replace the seven-segment display with an LCD showing ward, call type and time
Add more call types (emergency, bathroom cord, bed-exit sensor) and more wards using shift registers
Add alerts to other nurses (SMS, IoT or RF) when a call is not acknowledged
Enable the watchdog timer and add a battery backup
