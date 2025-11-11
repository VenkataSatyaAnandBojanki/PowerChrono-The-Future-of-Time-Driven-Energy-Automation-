#include <LPC214x.h>        // Header for LPC214x microcontroller (register definitions)
#include "delay.h"          // Custom delay header (for delay_ms function)

//========================== KEYPAD MATRIX DEFINITION ==========================//
// 4x4 keypad layout mapping (row x column)
// Each element represents the character corresponding to that key position
unsigned char keypad[4][4] = {
  {'1','2','3','%'},       // Row 0
  {'4','5','6','*'},       // Row 1
  {'7','8','9','-'},       // Row 2
  {'C','0','=','+'}        // Row 3
};

//========================== KEYPAD INITIALIZATION =============================//
void keypad_init(void) {
  IODIR1 &=  ~(0xF << 24);  // Configure P1.24–P1.27 as INPUT (columns)
                            // 0xF << 24 = 0xF000000, mask for bits 24–27

  IODIR1 |=   (0xF << 20);  // Configure P1.20–P1.23 as OUTPUT (rows)
                            // 0xF << 20 = 0x00F00000, mask for bits 20–23
}

//========================== KEYPAD SCANNING FUNCTION ==========================//
// This function returns the character of the key pressed on the 4x4 keypad
char key_pad(void) {
  int row, col;             // Loop counters for row and column scanning

  for(row=0; row<4; row++) {                       // Loop through each row
    IOSET1 = (0xF << 20);                          // Set all row lines HIGH
    IOCLR1 = (1 << (20 + row));                    // Drive the current row LOW (one at a time)

    for(col=0; col<4; col++) {                     // Check each column
      if(!(IOPIN1 & (1 << (24 + col)))) {          // If a column line reads LOW ? key pressed
        delay_ms(2);                               // Debounce delay (remove switch bounce)
        while(!(IOPIN1 & (1 << (24 + col))));      // Wait until the key is released
        delay_ms(2);                               // Short delay after release for stability
        return keypad[row][col];                   // Return the corresponding character
      }
    }
  }
  return 0; // Return 0 if no key is pressed
}
