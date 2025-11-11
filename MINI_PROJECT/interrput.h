#define EINT0_VIC_CHNO 14
#define EINT0_STATUS_LED  18//@p1.18


void Eint_Init(void);
void eint0_isr(void) __irq;
