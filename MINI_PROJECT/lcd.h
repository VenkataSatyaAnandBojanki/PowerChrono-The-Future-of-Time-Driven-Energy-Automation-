#include "delay.h"          // Header file for delay_ms() function
#include "types.h"          // Header file defining data types like u8, u16, etc.

//======================= LCD PIN CONNECTION MACROS ========================//
#define EN (1<<9)            // Enable pin connected to P0.9
#define RS (1<<8)            // Register Select pin connected to P0.8
#define PORTS (0xFF)         // Data bus connected to P0.0–P0.7 (8-bit mode)

//======================= SEND COMMAND TO LCD ==============================//
// Sends an instruction/command byte to LCD (like clear, cursor, etc.)
void lcd_cmd(char cmd){
	IO0PIN = (IO0PIN & 0XFFFFFF00) | cmd;   // Send command to lower 8 bits (P0.0–P0.7)
	IO0SET = EN;                            // Set EN = 1 (Enable HIGH)
	IO0CLR = RS;                            // RS = 0 (Command mode)
	delay_ms(1);                            // Small delay for LCD to latch data
	IO0CLR = EN;                            // EN = 0 (falling edge latches command)
	delay_ms(1);                            // Delay for LCD internal processing
}

//======================= SEND DATA TO LCD ================================//
// Sends a single character (data byte) to LCD (for display)
void lcd_data(char data){
	IO0PIN = (IO0PIN & 0XFFFFFF00) | data;  // Put data on P0.0–P0.7
	IO0SET = RS;                            // RS = 1 (Data mode)
  IO0SET = EN;                            // EN = 1 (Enable HIGH)
	delay_ms(1);                            // Small delay for data latch
	IO0CLR = EN;                            // EN = 0 (Falling edge writes data)
	delay_ms(1);                            // Short delay for LCD readiness
}

//======================= LCD INITIALIZATION ===============================//
// Initializes LCD in 8-bit mode with display ON and cursor OFF
void lcd_init(void){
	IO0DIR = 0X3FF;                         // Set P0.0–P0.9 as output (8 data + RS + EN)
	lcd_cmd(0x38);                          // Function set: 8-bit, 2-line, 5x7 font
	lcd_cmd(0X0C);                          // Display ON, cursor OFF
	lcd_cmd(0X06);                          // Entry mode: Auto increment cursor
	lcd_cmd(0X01);                          // Clear display
	//delay_ms(5);                          // (Optional) Additional delay if needed
}

//======================= DISPLAY STRING ON LCD ===========================//
// Displays a string of characters on LCD starting from current cursor
void lcd_str(char *str) { 
  while(*str)                               // Loop until null terminator
    lcd_data(*str++);                       // Send each character
}
	
//======================= SET CURSOR POSITION =============================//
// Sets the LCD cursor to a specific row and column (1-based index)
void lcd_goto(char row,char col){
	char pos;
	if(row ==1 )
		pos = (0x80) + (col-1);            // 1st line starts from DDRAM address 0x80
	else
		pos = (0xC0) + (col-1);            // 2nd line starts from DDRAM address 0xC0
	lcd_cmd(pos);                          // Send position command to LCD
}

//======================= LOAD CUSTOM CHARACTER INTO CGRAM =================//
// Writes custom 8-byte character pattern to one of 8 CGRAM locations (0–7)
void lcd_cgram(unsigned char location,unsigned char *data){
	unsigned char i;
	if(location<8){                         // Only 8 custom chars allowed
		lcd_cmd(0x40 + (location * 8));	    // Set CGRAM address (each char = 8 bytes)
	  for(i=0;i<8;i++){                     // Write 8 bytes pattern data
		  lcd_data(data[i]);
	  }
	  delay_ms(2);                          // Short delay for LCD to store pattern
	}
}
