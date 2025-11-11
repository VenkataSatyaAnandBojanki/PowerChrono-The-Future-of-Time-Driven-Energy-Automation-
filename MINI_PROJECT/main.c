#define LPC2138                    // Define target microcontroller LPC2138 (from LPC214x family)
#include<LPC214x.h>                // Include LPC214x header for register definitions
#include<stdlib.h>                 // Standard library for functions like atoi()
#include "rtc_define.h"            // Custom header containing RTC definitions/macros
#include "types.h"                 // Custom type definitions like u8, u32, etc.
#include "lcd.h"                   // LCD driver functions (lcd_cmd, lcd_data, lcd_str, etc.)
#include "keypad.h"                // Keypad handling functions (key_pad)
#include "interrput.h"             // Interrupt-related definitions

u8 emergency_exit =0;              // Global flag to indicate emergency exit from editing menu

#define check (key!=0 && key!='=' && key!='-' && key!='C')  // Macro for valid keypress (excluding control keys)

// External global variables used from other files
extern volatile unsigned char interrupt_flag; 
extern u32 ON_HOUR;
extern u32 ON_MIN;
extern u32 OFF_HOUR;
extern u32 OFF_MIN;

// Custom character bitmaps for LCD icons (8 bytes per pattern)
u8 bulb_icon_on[8]  = {0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x0A, 0x0A};  // Bulb ON icon
u8 bulb_icon_off[8] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x0A, 0x0A};  // Bulb OFF icon
u8 bell_icon_on[8]  = {0x04, 0x0E, 0x0E, 0x0E, 0x1F, 0x00, 0x04, 0x00};  // Bell ON icon
u8 bell_icon_off[8] = {0x04, 0x0A, 0x0A, 0x0A, 0x1F, 0x00, 0x04, 0x00};  // Bell OFF icon

//=========================== RTC Initialization ==============================
void rtc_init(void){
	CCR = 0x00;                     // Stop RTC counter (clear control register)
	CCR = RTC_RESTART;              // Restart RTC to ensure clean start
	CCR = 0x00;                     // Stop RTC again to configure prescalers
	
	CCR = RTC_CLKSRC;               // Select clock source (1 for LPC2148, 0 for LPC2129)
	
	PREINT = PREINT_VAL;            // Integer part of prescaler (for 1 sec tick at 32.768 kHz)
	PREFRAC = PREFRA_VAL;           // Fractional part of prescaler
	
	// Initialize RTC time and date registers
	SEC   = 0;                      // Seconds = 0
	MIN   = 00;                     // Minutes = 0
	HOUR  = 10;                     // Hour = 10 (default)
	DOM   = 28;                     // Date of Month = 28
	MONTH = 9;                      // Month = September
	YEAR  = 2025;                   // Year = 2025
	DOW   = 3;                      // Day of week = Wednesday (example)
	DOY   = 306;                    // Day of year = 306
	
 	CCR = RTC_ENABLE | RTC_CLKSRC ; // Enable RTC with selected clock source
}

// Array of strings for displaying day names
char *arr[] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};

//=========================== LCD Helper Function ==============================
void lcd_2digit(unsigned int val) { 
  lcd_data((val/10) + '0');       // Send tens place to LCD
  lcd_data((val%10) + '0');       // Send ones place to LCD
}

//=========================== BULB Control Logic ==============================
void BULB(void){
	IODIR1|=(1<<19);               // Set P1.19 as output (connected to bulb or LED)
	
	// Check if current time is within ON and OFF time
	if((HOUR >= ON_HOUR && HOUR <= OFF_HOUR) && (MIN >= ON_MIN && MIN < OFF_MIN)){	
		IOSET1=1<<19;              // Turn ON bulb
		lcd_data(6);               // Display custom character (bulb ON icon)
	}
	else 
		IOCLR1=1<<19;              // Turn OFF bulb
		lcd_data(7);               // Display custom character (bulb OFF icon)
}

//=========================== BELL Icon Display ==============================
void BELL(void){
  if(ON_HOUR || ON_MIN || OFF_HOUR || OFF_MIN){  // If any ON/OFF time is set
		lcd_data(4);              // Show bell ON icon
	}
	else{
		//lcd_data(5);            // (optional) Show bell OFF icon if desired
	}
}

//=========================== RTC EDIT MENU ==============================
void RTC_EDIT(void){
	char key,hour[3],min[3],sec[3],date[3],day[2],mon[3],year[5];  // Local buffers for time/date entries

	EDIT :  lcd_cmd(0x01);             // Clear LCD
	        lcd_goto(1,1);             // Go to first line
	        lcd_str("1.H 2.M 3.S 4.D"); // Display main edit menu
          lcd_goto(2,1);
          lcd_str("5.DY 6.M 7.Y 8.E");  // Display remaining edit options
          
	 while(1){
		 key=key_pad();               // Wait for key press
		 if(key!=0){                  // If a key is pressed
			 switch(key){             // Check which option was selected

//=========================== EDIT HOUR ==============================
				 case '1' : 
					 rtc_hour:lcd_cmd(0x01);       // Clear LCD
				            lcd_str("ENTER : HOUR"); // Prompt user to enter hour 
				            lcd_goto(2,1);

				         while(1){                 // Wait for first digit
									 First:key=key_pad();
									 if(check){        // Valid key check
				            hour[0]=key;           // Store first digit
				            lcd_data(hour[0]);     // Display it on LCD
										break; 
									 }else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;} // Emergency exit
								 }
									 while(1){       // Wait for second digit
										 second:key=key_pad();
										if(key != 0){
											if(key == '-'){    // Backspace operation
												lcd_goto(2,1);
												lcd_data(' ');
												lcd_goto(2,1);
												goto First;
											}else if(key == '='){ hour[1]='\0'; goto h;} // Enter pressed
											 else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											hour[1] = key;      // Store 2nd digit
											lcd_data(hour[1]);  // Display 2nd digit
											break;
										}
										}
										while(1){     // Wait for confirmation or error handling
											key = key_pad();
											if(key != 0){
												if(key == '='){        // '=' means confirm entry
													hour[2] = '\0';
													h:if(atoi(hour)>=0 && atoi(hour)<24){ // Validate 0–23
													HOUR =atoi(hour);   // Update RTC hour register
													goto EDIT;}         // Go back to main menu
													else{               // Invalid hour
														lcd_cmd(0x01);
														lcd_str("!!! ERROR !!!");
														lcd_goto(2,1);
														lcd_str("ENTER : 1.BACK");
														while(1){
														key=key_pad();
														if(key == '1')
															goto rtc_hour;  // Retry input
												 }
													}
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
												 else if(key == '-'){   // Backspace again
													lcd_goto(2,2);
													lcd_data(' ');
													lcd_goto(2,2);
													goto second;
												}
											}
										}//case 1

//=========================== EDIT MINUTES ==============================
				 case '2': 
					 rtc_min:  lcd_cmd(0x01);
				            lcd_str("ENTER : MIN"); 
				            lcd_goto(2,1);
				         while(1){
									 First_min:key=key_pad();
									 if(check){
				            min[0]=key;
				            lcd_data(min[0]);
										break; 
									 }else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
								 }
									 while(1){
										 second_min:key=key_pad();
										if(key != 0){
											if(key == '-'){
												lcd_goto(2,1);
												lcd_data(' ');
												lcd_goto(2,1);
												goto First_min;
											}else if(key == '='){ min[1]='\0'; goto m;}
											else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											min[1] = key;
											lcd_data(min[1]);
											break;
										}
										}
										while(1){
											key = key_pad();
											if(key != 0){
												if(key == '='){
													min[2] = '\0';
													m:if(atoi(min)>=0 && atoi(min)<=59){
													MIN =atoi(min);  // Update RTC minutes
													goto EDIT;}
													else{            // Invalid minutes
														lcd_cmd(0x01);
														lcd_str("!!! ERROR !!!");
														lcd_goto(2,1);
														lcd_str("ENTER : 1.BACK");
														while(1){
														key=key_pad();
														if(key == '1')
															goto rtc_min;
												 }
													}}
												else if(key == '-'){
													lcd_goto(2,2);
													lcd_data(' ');
													lcd_goto(2,2);
													goto second_min;
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											}
										}//case2			

//=========================== EDIT SECONDS ==============================
          case '3': 
					 rtc_sec:  lcd_cmd(0x01);
				            lcd_str("ENTER : SEC"); 
				            lcd_goto(2,1);
				         while(1){
									 First_sec:key=key_pad();
									 if(check){
				            sec[0]=key;
				            lcd_data(sec[0]);
										break; 
									 }else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
								 }
									 while(1){
										 second_sec:key=key_pad();
										if(key != 0){
											if(key == '-'){
												lcd_goto(2,1);
												lcd_data(' ');
												lcd_goto(2,1);
												goto First_sec;
											}else if(key == '='){ sec[1]='\0'; goto s;}
										 	else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											sec[1] = key;
											lcd_data(sec[1]);
											break;
										}
										}
										while(1){
											key = key_pad();
											if(key != 0){
												if(key == '='){
													sec[2] = '\0';
													s:	if(atoi(sec)>=0 && atoi(sec)<=59){
													SEC =atoi(sec);    // Update seconds
													goto EDIT;}
													else{              // Invalid seconds
														lcd_cmd(0x01);
														lcd_str("!!! ERROR !!!");
														lcd_goto(2,1);
														lcd_str("ENTER : 1.BACK");
														while(1){
														key=key_pad();
														if(key == '1')
															goto rtc_sec;
												 }
													}}
												else if(key == '-'){
													lcd_goto(2,2);
													lcd_data(' ');
													lcd_goto(2,2);
													goto second_sec;
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											}
										}//case3	

//=========================== EDIT DATE ==============================
           case '4': 
					 rtc_date:  lcd_cmd(0x01);
				            lcd_str("ENTER : DATE"); 
				            lcd_goto(2,1);
				         while(1){
									 First_date:key=key_pad();
									 if(check){
				            date[0]=key;
				            lcd_data(date[0]);
										break; 
									 }else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
								 }
									 while(1){
										 second_date:key=key_pad();
										if(key != 0){
											if(key == '-'){
												lcd_goto(2,1);
												lcd_data(' ');
												lcd_goto(2,1);
												goto First_date;
											}else if(key == '='){ date[1]='\0'; goto d;}
											else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											date[1] = key;
											lcd_data(date[1]);
											break;
										}
										}
										while(1){
											key = key_pad();
											if(key != 0){
												if(key == '='){
													date[2] = '\0';
													d:if(atoi(date)>0 && atoi(date)<=31){
													DOM =atoi(date);  // Update date
													goto EDIT;}
													else{             // Invalid date
														lcd_cmd(0x01);
														lcd_str("!!! ERROR !!!");
														lcd_goto(2,1);
														lcd_str("ENTER : 1.BACK");
														while(1){
														key=key_pad();
														if(key == '1')
															goto rtc_date;
												 }
													}}
												else if(key == '-'){
													lcd_goto(2,2);
													lcd_data(' ');
													lcd_goto(2,2);
													goto second_date;
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											}
										}//case4		

//=========================== EDIT DAY OF WEEK ==============================
					 case '5' :
						 rtc_day : lcd_cmd(0x01);
					             lcd_str("ENTER : Day"); 
					             delay_s(1);
					             lcd_cmd(0x01);
					             lcd_str("0.Su 1.M 2.T 3.W"); // Day options
					             lcd_goto(2,1);
					             lcd_str("4.T 5.F 6.Sa");
					             while(1){
												 key=key_pad();
												 if(check){
													 day[0]=key;
													 day[1]='\0';
													 if(atoi(day)>=0 && atoi(day)<7){ // Valid day 0–6
														 DOW = atoi(day);
														 goto EDIT;}
													 else{
														 lcd_cmd(0x01);
														 lcd_str("!!! ERROR !!!");
														 goto rtc_day;
													 }
												 }else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											 }//case 5

//=========================== EDIT MONTH ==============================
						case '6': 
					 rtc_mon:  lcd_cmd(0x01);
				            lcd_str("ENTER : MONTH"); 
				            lcd_goto(2,1);
				         while(1){
									 First_mon:key=key_pad();
									 if(check){
				            mon[0]=key;
				            lcd_data(mon[0]);
										break; }else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
									 }
									 while(1){
										 second_mon:key=key_pad();
										if(key != 0){
											if(key == '-'){
												lcd_goto(2,1);
												lcd_data(' ');
												lcd_goto(2,1);
												goto First_mon;
											}else if(key== '='){mon[1]='\0'; goto M;}
											else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											mon[1] = key;
											lcd_data(mon[1]);
											break;
										}
										}
										while(1){
											key = key_pad();
											if(key != 0){
												if(key == '='){
													mon[2] = '\0';
													M:		if(atoi(mon)>0 && atoi(mon)<=12){
													MONTH =atoi(mon); // Update month
													goto EDIT;}
													else{             // Invalid month
														lcd_cmd(0x01);
														lcd_str("!!! ERROR !!!");
														lcd_goto(2,1);
														lcd_str("ENTER : 1.BACK");
														while(1){
														key=key_pad();
														if(key == '1')
															goto rtc_mon;
												 }
													}}
												else if(key == '-'){
													lcd_goto(2,2);
													lcd_data(' ');
													lcd_goto(2,2);
													goto second_mon;
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											}
										}//case6 

//=========================== EDIT YEAR ==============================
						case '7':
						rtc_year:  lcd_cmd(0x01);
				               lcd_str("ENTER : YEAR"); 
				               lcd_goto(2,1);
										while(1){
				  	First_year: key=key_pad();
					             if(check){
                       year[0]=key;
                       lcd_data(year[0]);	
                       break;}
											 else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
										 }
                   while(1){
										  second_year:key=key_pad();
										if(key != 0){
											if(key == '-'){
												lcd_goto(2,1);
												lcd_data(' ');
												lcd_goto(2,1);
												goto First_year;
											}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											year[1] = key;
											lcd_data(year[1]);
											break;
										}
										}
									 while(1){
										 Third_year:key=key_pad();
										 if(key!=0){
											 if(key == '-'){
												 lcd_goto(2,2);
												 lcd_data(' ');
												 lcd_goto(2,2);
												 goto second_year;
											 }else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											 year[2]= key;
											 lcd_data(year[2]);
											 break;
										 }
									 }
									 while(1){
										 Fourth_year:key=key_pad();
										 if(key !=0){
											 if(key == '-'){
												 lcd_goto(2,3);
												 lcd_data(' ');
												 lcd_goto(2,3);
												 goto Third_year;
											 }else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											 year[3] = key;
											 lcd_data(year[3]);
											 break;
										 }
									 }
									 while(1){
											key = key_pad();
											if(key != 0){
												if(key == '='){
													year[4] = '\0';
													if(atoi(year)<=2024){ 
													 lcd_cmd(0x01);lcd_str("! WARNING !");
													 lcd_goto(2,1);
													 lcd_str("GOING TO PAST");
													 YEAR =atoi(year);
													 delay_s(5);
													 goto EDIT;}
													if(atoi(year)>0 && atoi(year)<=4095){
													YEAR =atoi(year);   // Update year
													goto EDIT;
													}
													else{
														lcd_cmd(0x01);
														lcd_str("!!! ERROR !!!");
														lcd_goto(2,1);
														lcd_str("ENTER : 1.BACK");
														while(1){
														key=key_pad();
														if(key == '1')
															goto rtc_year;
												   }
													}
												}
												else if(key == '-'){
													lcd_goto(2,4);
													lcd_data(' ');
													lcd_goto(2,4);
													goto Fourth_year;
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											}
										} //case7

//=========================== EXIT FROM EDIT MODE ==============================
						case '8' : 
              return;                  // Exit menu normally
						case 'C' :lcd_cmd(0x01); emergency_exit =1; return; // Emergency exit via 'C'						
									 
}//switch
}//waiting for key
		 }//while
	 }//rtc_edit

//============================= RTC ON/OFF TIME SETUP FUNCTION =============================//
void RTC_ON_OFF(void){
	 char key,hour[3],min[3];                   // Variables to hold key input and temporary hour/minute strings
rtc_on_off:	 lcd_cmd(0x01);                      // Clear LCD screen
	           lcd_str("1.ON TIME 3.EXIT");       // Display first menu line
	           lcd_goto(2,1);                    // Go to next line
	           lcd_str("2.OFF TIME");            // Display option to set OFF time
	  while(1){                                  // Main infinite loop
		key=key_pad();                           // Read key from keypad
		if(key!=0 && key!='=' && key!='-'){      // Ignore invalid control keys
			switch(key){                         // Check which option pressed
				case '1' :		                // If user selects ON TIME
					ON:       lcd_cmd(0x01);     // Clear LCD
				            lcd_str("1.HOUR   3.EXIT"); // Submenu for ON TIME
				            lcd_goto(2,1);
				            lcd_str("2.MIN");   // Display minute option
				            while(1){            // Submenu loop
										key=key_pad();
				            if(key!='\0'){       // Wait for valid key
										switch(key){    // Inside ON TIME submenu
					               case '1':    // Edit ON HOUR
					           rtc_onhour:lcd_cmd(0x01);
				                  lcd_str("ENTER : HOUR"); 
				                   lcd_goto(2,1);
				         while(1){
									 Firston:key=key_pad();
									 if(check){              // Valid key input
				                 hour[0]=key;               // Store first digit
				                lcd_data(hour[0]);          // Display first digit
										break; 
									 }else if(key=='C'){     // Emergency exit
										emergency_exit=1;
										lcd_cmd(0x01);
										return;
								 }
								 }
									 while(1){               // Wait for second digit
										 secondon:key=key_pad();
										if(key != 0){
											if(key == '-'){  // Backspace logic
												lcd_goto(2,1);
												lcd_data(' ');
												lcd_goto(2,1);
												goto Firston;
											}else if(key== '='){ hour[1]='\0'; goto h_on;} // Confirm entry
											else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											hour[1] = key;
											lcd_data(hour[1]);
											break;
										}
										}
										while(1){             // Wait for confirmation '='
											key = key_pad();
											if(key != 0){
												if(key == '='){ // Confirm entry
													hour[2] = '\0';
													h_on:	if(atoi(hour)>=0 && atoi(hour)<24){ // Validate 0–23 hour
												u32	temp = atoi(hour);    // Convert string to integer
													if((OFF_HOUR ==0) && (OFF_MIN ==0)){ // If OFF time not set yet
													ON_HOUR =atoi(hour);    // Assign ON hour
													goto ON ;               // Return to ON menu
													}
											        else if(temp<=OFF_HOUR) { // Ensure ON time < OFF time
													 ON_HOUR=temp;
													 goto ON;
													 }
													 else{                    // Invalid condition
												     lcd_cmd(0x01);
												    lcd_str("INVALID_TIME");
												     delay_s(1);
												     lcd_cmd(0x01);
													 goto rtc_onhour;
													}
													}// Valid range
													else{                    // Invalid input
														lcd_cmd(0x01);
														lcd_str("!!! ERROR !!!");
														lcd_goto(2,1);
														lcd_str("ENTER : 1.BACK");
														while(1){
														key=key_pad();
														if(key == '1') goto rtc_onhour;
												 }
													}}
												else if(key == '-'){        // Backspace
													lcd_goto(2,2);
													lcd_data(' ');
													lcd_goto(2,2);
													goto secondon;
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											}
										}//case 1 end (ON hour)

						case '2' :	  // Set ON MINUTES
										rtc_onmin:  lcd_cmd(0x01);
				            lcd_str("ENTER : MIN"); 
				            lcd_goto(2,1);
				         while(1){
									 First_minon:key=key_pad();
									 if(check){
				            min[0]=key;
				            lcd_data(min[0]);
										break; 
									 }else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
								 }
									 while(1){
										 second_minon:key=key_pad();
										if(key != 0){
											if(key == '-'){
												lcd_goto(2,1);
												lcd_data(' ');
												lcd_goto(2,1);
												goto First_minon;
											}else if(key== '='){ min[1]='\0'; goto m_on;}
										 	else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											min[1] = key;
											lcd_data(min[1]);
											break;
										}
										}
										while(1){
											key = key_pad();
											if(key != 0){
												if(key == '='){     // Confirm entry
													min[2] = '\0';
													m_on:if(atoi(min)>=0 && atoi(min)<=59){ // Validate minutes
										        	u32	temp2 = atoi(min);
													if((OFF_HOUR ==0) && (OFF_MIN ==0)){  // If OFF time not set
													ON_MIN =atoi(min);
													goto ON ;
													}
											        else if(temp2<OFF_MIN) {             // Ensure ON<OFF
													 ON_MIN=temp2;
													 goto ON;
													 }
													 else{
												     lcd_cmd(0x01);
												    lcd_str("INVALID_TIME");
												     delay_s(1);
												     lcd_cmd(0x01);
													 goto rtc_onmin;
													}
														}
													else{ // Invalid minute
														lcd_cmd(0x01);
														lcd_str("!!! ERROR !!!");
														lcd_goto(2,1);
														lcd_str("ENTER : 1.BACK");
														while(1){
														key=key_pad();
														if(key == '1') goto rtc_onmin;
												 }
													}}
												else if(key == '-'){
													lcd_goto(2,2);
													lcd_data(' ');
													lcd_goto(2,2);
													goto second_minon;
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											}
										}//case2 end			
						    case '3':	goto rtc_on_off;   // Exit ON time setup
						    case 'C' :emergency_exit=1; lcd_cmd(0x01); return; // Emergency exit
										}// on time switch end
										}// on time if end
									}//on time while end
								
						case '2':        // USER SELECTS OFF TIME
							OFF:    lcd_cmd(0x01);
				            lcd_str("1.HOUR   3.EXIT"); 
				            lcd_goto(2,1);
				            lcd_str("2.MIN"); 
				            while(1){
										key=key_pad();
				            if(key!=0){
										switch(key){
					  case '1':  // SET OFF HOUR
					 rtc_offhour: lcd_cmd(0x01);
				            lcd_str("ENTER : HOUR"); 
				            lcd_goto(2,1);
				         while(1){
									 Firstoff:key=key_pad();
									 if(check){
				            hour[0]=key;
				            lcd_data(hour[0]);
										break; 
									 }else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
								 }
									 while(1){
										 secondoff:key=key_pad();
										if(key != 0){
											if(key == '-'){
												lcd_goto(2,1);
												lcd_data(' ');
												lcd_goto(2,1);
												goto Firstoff;
											}else if(key== '='){ hour[1]='\0'; goto h_off;}
											 else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											hour[1] = key;
											lcd_data(hour[1]);
											break;
										}
										}
										while(1){
											key = key_pad();
											if(key != 0){
												if(key == '='){
													hour[2] = '\0';
													h_off:	if(atoi(hour)>=0 && atoi(hour)<24){ // Validate hour
													int temp=atoi(hour);
													if(temp>=ON_HOUR){  // OFF hour must be >= ON hour
													OFF_HOUR = temp;
													goto OFF;} 
													else{
													 lcd_cmd(0x01);
													 lcd_str("INVALID_OFFTIME");
													 delay_s(1);
													 lcd_cmd(0x01);
													 goto rtc_offhour;
													 }

													}
													else{
														lcd_cmd(0x01);
														lcd_str("!!! ERROR !!!");
														lcd_goto(2,1);
														lcd_str("ENTER 1.BACK");
														while(1){
														key=key_pad();
														if(key == '1')
															goto rtc_offhour;
												 }
													}}
												else if(key == '-'){
													lcd_goto(2,2);
													lcd_data(' ');
													lcd_goto(2,2);
													goto secondoff;
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											}
										}//case 1 end (OFF hour)
						case '2' :	 // SET OFF MINUTES
										rtc_offmin:  lcd_cmd(0x01);
				            lcd_str("ENTER : MIN"); 
				            lcd_goto(2,1);
				         while(1){
									 First_minoff:key=key_pad();
									 if(check){
				            min[0]=key;
				            lcd_data(min[0]);
										break; 
									 }else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
								 }
									 while(1){
										 second_minoff:key=key_pad();
										if(key != 0){
											if(key == '-'){
												lcd_goto(2,1);
												lcd_data(' ');
												lcd_goto(2,1);
												goto First_minoff;
											}else if(key== '='){ min[1]='\0'; goto m_off;}
										 	else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											min[1] = key;
											lcd_data(min[1]);
											break;
										}
										}
										while(1){
											key = key_pad();
											if(key != 0){
												if(key == '='){
													min[2] = '\0';
													m_off:	if(atoi(min)>=0 && atoi(min)<=59){ // Validate
													int temp2=atoi(min);
													if(temp2>ON_MIN){    // OFF minute > ON minute
													OFF_MIN = temp2;
													goto OFF;}
													else{
														lcd_cmd(0x01);
														lcd_str("INVALID_OFFTIME");
														delay_s(1);
														lcd_cmd(0x01);
														goto rtc_offmin;
													}
													}
													else{
														lcd_cmd(0x01);
														lcd_str("!!! ERROR !!!");
														lcd_goto(2,1);
														lcd_str("ENTER : 1.BACK");
														while(1){
														key=key_pad();
														if(key == '1') goto rtc_offmin;
												 }
													}}
												else if(key == '-'){
													lcd_goto(2,2);
													lcd_data(' ');
													lcd_goto(2,2);
													goto second_minoff;
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											}
										}//case2 end			
						    case '3':	goto rtc_on_off;  // Exit OFF time setup
						    case 'C': emergency_exit=1;lcd_cmd(0x01); return;
										}// off time switch end
										}// off time if end
									}//off time while end

						case '3' : return;	 // EXIT from RTC_ON_OFF	
           case 'C': emergency_exit=1;lcd_cmd(0x01); return;						
			}//MAIN SWITCH END 
		}// FUN IF END
	}//FUN WHILE END
}//FUN END


//============================= DISPLAY CURRENT ON/OFF TIMES =============================//
void ON_OFF_BUTTON(void){
	lcd_cmd(0x01);                 // Clear LCD
	lcd_goto(1,1);                 // Go to line 1
	lcd_str("ON_TIME  ");          // Display ON time
	lcd_2digit(ON_HOUR);           // Display hour
  lcd_data(':');
  lcd_2digit(ON_MIN);             // Display minutes
		
	lcd_goto(2,1);
	lcd_str("OFF_TIME ");          // Display OFF time
	lcd_2digit(OFF_HOUR);
  lcd_data(':');
	lcd_2digit(OFF_MIN);
	delay_s(2);                    // Hold display for 2 seconds
	lcd_cmd(0x01);
	return;
}

//============================= MAIN OPTION MENU =============================//
void option(void){
    	char key;
	    int i=(120000*3);            // Rough loop count for timeout delay
	START:  lcd_cmd(0x01);
			lcd_goto(1,1);
			lcd_str("1.E.TIME");      // Edit RTC Time
			lcd_goto(2,1);
			lcd_str("2.E.ON_OFF_TIME");// Edit ON/OFF Time
			lcd_goto(1,11);
			lcd_str("3.BACK");        // Return option
	     while(i--){                 // Wait for user input
	     key=key_pad();
		 if(key!=0){
	    switch(key){
			 case '1' : RTC_EDIT();  // Call RTC time edit function
                 if(emergency_exit){emergency_exit=0;lcd_cmd(0x01); return;}
  								 goto START;
			 case '2' : RTC_ON_OFF(); // Call RTC ON/OFF time function
								 if(emergency_exit){emergency_exit=0;lcd_cmd(0x01); return;}
								 goto START;
			 case '3' : lcd_cmd(0x01);return; // Return to main
			 case 'C': lcd_cmd(0x01);return;} // Cancel and return
}
}
lcd_cmd(0x01);
}


//============================= MAIN LCD DISPLAY FUNCTION =============================//
void main_lcd(void){
		lcd_goto(1,1);
		lcd_2digit(HOUR);            // Display current hour
		lcd_data(':');
		lcd_2digit(MIN);             // Display current minute
		lcd_data(':');
		lcd_2digit(SEC);             // Display seconds
		lcd_goto(1,10);
		BELL();                      // Show bell icon
		lcd_goto(1,12);
		lcd_str("DEV:");             // Device label
		lcd_goto(1,16);
		BULB();                      // Show bulb status
		
		lcd_goto(2,1);
		lcd_2digit(DOM);             // Display date
		lcd_data('/');
		lcd_2digit(MONTH);           // Display month
		lcd_data('/');
		lcd_data((YEAR/1000)%10 + '0'); // Display year digits
		lcd_data((YEAR/100)%10 +'0');
		lcd_data((YEAR/10)%10 + '0');
		lcd_data((YEAR%10) + '0');
		lcd_str(" ");
		lcd_str(arr[DOW]);           // Display day of week (from array)
		return;
}


//============================= MAIN FUNCTION =============================//
int main(){
	PINSEL0=0;                      // Configure all pins as GPIO
	lcd_init();                     // Initialize LCD module
	rtc_init();                     // Initialize RTC
	keypad_init();                  // Initialize keypad
	lcd_cgram(4,bell_icon_on);      // Load custom icons to LCD CGRAM
	lcd_cgram(5,bell_icon_off);
	lcd_cgram(6,bulb_icon_on);
	lcd_cgram(7,bulb_icon_off);
	Eint_Init();                    // Initialize external interrupts
	while(1){                       // Infinite main loop
		main_lcd();                 // Continuously display RTC values
		if(!(IO0PIN & 0X0400)){     // Check button input (connected to P0.10)
			ON_OFF_BUTTON();        // Display ON/OFF times when pressed
		}
		if(interrupt_flag){          // Check if interrupt occurred
			interrupt_flag = 0;      // Clear interrupt flag
			option();                // Enter option menu
			lcd_cmd(0x01);           // Clear LCD after returning
		}
	}
}


