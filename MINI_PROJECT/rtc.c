#define LPC2138
#include<LPC214x.h>
#include<stdlib.h>
#include "rtc_define.h"
#include "types.h"
#include "lcd.h"
#include "keypad.h"
#include "interrput.h"

extern u32 ON_HOUR;
extern u32 ON_MIN;
extern u32 OFF_HOUR;
extern u32 OFF_MIN;

void RTC_ON_OFF(void){
	 char key,hour[3],min[3];
rtc_on_off:	 lcd_cmd(0x01);
	           lcd_str("1.ON TIME 3.EXIT");
	           lcd_goto(2,1);
	           lcd_str("2.OFF TIME");
	  while(1){
		key=key_pad();
		if(key!=0 && key!='=' && key!='-'){
			switch(key){
				case '1' :		
					ON:       lcd_cmd(0x01);
				            lcd_str("1.HOUR   3.EXIT"); 
				            lcd_goto(2,1);
				            lcd_str("2.MIN"); 
				            while(1){
										key=key_pad();
				            if(key!=0){
										switch(key){
					               case '1':
					           rtc_onhour:lcd_cmd(0x01);
				                  lcd_str("ENTER : HOUR"); 
				                   lcd_goto(2,1);
				            //lcd_cmd(0x0E);

				         while(1){
									 Firston:key=key_pad();
									 if(check){
				                 hour[0]=key;
				                lcd_data(hour[0]);
										break; 
									 }else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
								 }
									 while(1){
										 secondon:key=key_pad();
										if(key != 0){
											if(key == '-'){
												lcd_goto(2,1);
												lcd_data(' ');
												lcd_goto(2,1);
												goto Firston;
											}else if(key== '='){ hour[1]='\0'; goto h_on;}
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
													h_on:	if(atoi(hour)>=0 && atoi(hour)<24){
													ON_HOUR =atoi(hour);
													goto ON ;
													}
													else{
														lcd_cmd(0x01);
														lcd_str("!!! ERROR !!!");
														lcd_goto(2,1);
														lcd_str("ENTER : 1.BACK");
														while(1){
														key=key_pad();
														if(key == '1')
															goto rtc_onhour;
												 }
													}}
												else if(key == '-'){
													lcd_goto(2,2);
													lcd_data(' ');
													lcd_goto(2,2);
													goto secondon;
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											}
										}//case 1
						case '2' :	
										rtc_onmin:  lcd_cmd(0x01);
				            lcd_str("ENTER : MIN"); 
				            lcd_goto(2,1);
				            //lcd_cmd(0x0E);

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
												if(key == '='){
													min[2] = '\0';
													m_on:if(atoi(min)>=0 && atoi(min)<=59){
													ON_MIN =atoi(min);
													goto ON;}
													else{
														lcd_cmd(0x01);
														lcd_str("!!! ERROR !!!");
														lcd_goto(2,1);
														lcd_str("ENTER : 1.BACK");
														while(1){
														key=key_pad();
														if(key == '1')
															goto rtc_onmin;
												 }
													}}
												else if(key == '-'){
													lcd_goto(2,2);
													lcd_data(' ');
													lcd_goto(2,2);
													goto second_minon;
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											}
										}//case2			
						    case '3':	goto rtc_on_off;
						    case 'C' :emergency_exit=1; lcd_cmd(0x01); return;
										}// on time switch
										}// on time if
									}//on time while
								
						case '2':
							OFF:    lcd_cmd(0x01);
				            lcd_str("1.HOUR   3.EXIT"); 
				            lcd_goto(2,1);
				            lcd_str("2.MIN"); 
				            while(1){
										key=key_pad();
				            if(key!=0){
										switch(key){
					  case '1':
					 rtc_offhour: lcd_cmd(0x01);
				            lcd_str("ENTER : HOUR"); 
				            lcd_goto(2,1);
				            //lcd_cmd(0x0E);

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
													h_off:	if(atoi(hour)>=0 && atoi(hour)<24){
													int temp=atoi(hour);
													if(temp>=ON_HOUR){
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
										}//case 1
						case '2' :	
										rtc_offmin:  lcd_cmd(0x01);
				            lcd_str("ENTER : MIN"); 
				            lcd_goto(2,1);
				            //lcd_cmd(0x0E);

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
													m_off:	if(atoi(min)>=0 && atoi(min)<=59){
													int temp2=atoi(min);
													if(temp2>ON_MIN){
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
														if(key == '1')
															goto rtc_offmin;
												 }
													}}
												else if(key == '-'){
													lcd_goto(2,2);
													lcd_data(' ');
													lcd_goto(2,2);
													goto second_minoff;
												}else if(key=='C'){emergency_exit=1;lcd_cmd(0x01); return;}
											}
										}//case2			
						    case '3':	goto rtc_on_off;
						    case 'C': emergency_exit=1;lcd_cmd(0x01); return;
										}// off time switch
										}// off time if
									}//off time while

						case '3' : return;						
           case 'C': emergency_exit=1;lcd_cmd(0x01); return;						
			}//MAIN SWITCH END 
		}// FUN IF END
	}//FUN WHILE END
}//FUN END