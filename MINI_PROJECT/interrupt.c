#include <LPC21xx.h>         // Header for LPC21xx series (contains register definitions)
#include "types.h"           // Custom header file defining data types like u32, u8, etc.
#include "interrput.h"       // Custom interrupt header (likely contains macros like EINT0_VIC_CHNO)

//========================= GLOBAL VARIABLES =========================//
u32 ON_HOUR;                 // Variable to store ON time hour
u32 ON_MIN;                  // Variable to store ON time minutes
u32 OFF_HOUR;                // Variable to store OFF time hour
u32 OFF_MIN;                 // Variable to store OFF time minutes

void lcd_cmd(char);          // Prototype for LCD command function (used elsewhere)
u32 interrupt_flag = 0;      // Global flag to indicate interrupt occurrence


//========================= EXTERNAL INTERRUPT INITIALIZATION =========================//
void Eint_Init(void){
	//CfgPortPinFunc(0,16,PIN_FUNC2);   // Optional helper macro to configure P0.16 as EINT0 (commented)
	PINSEL1=0X01;                      // Select alternate function for P0.16 ? EINT0 (bits 0-1 = 01)

	VICIntEnable = 1 << EINT0_VIC_CHNO;  // Enable EINT0 interrupt channel in VIC interrupt enable register
  VICVectCntl0 = (1 << 5) | EINT0_VIC_CHNO; // Enable slot 0 in VIC and assign it to EINT0 interrupt
	VICVectAddr0 = (unsigned int)eint0_isr;  // Set ISR address for EINT0 in VIC vector slot 0

  EXTMODE  = 1 << 0;             // Configure EINT0 as edge-sensitive (bit 0 ? 1)
  EXTPOLAR = 0;                  // Select falling edge triggering (0 = falling, 1 = rising)
  EXTINT   = 1 << 0;             // Clear any pending EINT0 interrupt flag before enabling
}


//========================= EXTERNAL INTERRUPT SERVICE ROUTINE =========================//
void eint0_isr(void) __irq {
    interrupt_flag = 1;          // Set global flag to signal main program that interrupt occurred
    EXTINT = 1 << 0;             // Clear EINT0 interrupt flag (acknowledge the external interrupt)
    VICVectAddr = 0;             // Write to VICVectAddr to signal end of ISR to the VIC controller
}
