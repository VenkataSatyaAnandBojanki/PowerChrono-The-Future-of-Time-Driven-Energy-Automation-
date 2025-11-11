#ifndef RTC_DEFINE
#define RTC_DEFINE

#define RTC_ENABLE (1<<0) // RTC ENABLE 
#define RTC_RESTART (1<<1)// RTC RESTART

#define FOSC 12000000
#define CCLK (FOSC*5)
#define PCLK (CCLK/4)


#if defined(LPC2148)|| defined(LPC2138)
#define RTC_CLKSRC (1<<4) // CLOCK SROUCES HIGH
#define PREINT_VAL ((PCLK/32768)-1)
#define PREFRA_VAL (PCLK-(PREINT_VAL +1)*32768)
#elif defined(LPC2129) || defined(LPC2124)
#define RTC_CLKSRC 0
#else
 #error "Please define target MCU: LPC2148, LPC2138, LPC2129, or LPC2124"
#endif

#endif

