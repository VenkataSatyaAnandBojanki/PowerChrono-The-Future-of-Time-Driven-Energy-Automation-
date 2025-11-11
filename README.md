# PowerChrono-The-Future-of-Time-Driven-Energy-Automation-
PowerChrono is an intelligent, time-based power management system designed to  optimize energy usage in industrial environments. The project focuses on automating electrical devices and loads according to predefined time schedules, ensuring efficient energy distribution, reduced wastage, and improved operational reliability. 
Features
Displays real-time clock (RTC) information (date, time) on a 16x2 LCD.​

Allows users to set or modify RTC and device ON/OFF timings using a 4x4 matrix keypad.​

Controls a device (e.g., LED or relay) according to programmed ON/OFF schedule.​

Includes menu-based user interaction using external interrupt and switches.​

Input validation for correct time/date and timings to ensure reliable operation.​

Hardware Connections
LCD Display:

Data Pins: P0.0 to P0.7

Register Select (RS): P0.8

Enable (EN): P0.9​

4x4 Keypad:

Connected to P1.20 to P1.27​

Interrupt Switch:

Connected to P0.16 (used to access menu for editing on-the-fly)​

Device Control Switch (On-Time/Off-Time Check):

Connected to P0.10​

Software Requirements
Embedded C (Keil MDK recommended)​

Flash Magic for programming LPC21xx

Proteus or any other simulation software (optional)​

How It Works
Initialization: LCD, keypad, RTC, and interrupt are initialized in main().​

Super Loop: Continuously reads RTC; displays date/time on LCD.​

Device Control: Turns device ON/OFF based on current time compared with programmed ON/OFF time.​

Editing Mode: Pressing the interrupt switch (P0.16) opens a menu:

EDIT RTC Info: Edit time/date values (validation included)

EDIT ON/OFF Time: Set new ON/OFF timings for device (validation included)

EXIT: Resume normal operation​

Navigation: Keypad is used to navigate menu and input values. Invalid entries prompt retry messages.​

Pin Mapping               Function	LPC2129 Pin(s)
LCD Data (D0-D7)	        P0.0 - P0.7
LCD RS	                  P0.8
LCD EN	                  P0.9
Keypad Rows/Cols         	P1.20 - P1.27
Interrupt Switch/Menu	    P0.16
ON/OFF Time Switch	      P0.10

Usage Instructions
Power up the system. The LCD shows current time and date.

Use the keypad to set/edit time, date, ON/OFF timings via the on-screen menu.

Press the switch connected to P0.16 anytime to enter editing mode; use keypad to make selections.

Device operation is automatically controlled by your programmed schedule.

Code Structure
Main loop for RTC display and device control.

Interrupt-based menu for editing parameters.

Robust input validation for all user entries.

Simulation and Testing
Test hardware connections with Proteus or on a breadboard.

Confirm correct mapping of GPIO pins before flashing firmware.

Validate ON/OFF control logic in response to RTC and user inputs.
