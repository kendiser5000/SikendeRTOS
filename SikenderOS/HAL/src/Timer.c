/** @file Timer.c
 * @brief Periodic Timer setup for TM4c123
 * @author Sijin Woo (https://github.com/SijWoo)
 */


#include "Timer.h"
#include "tm4c123gh6pm.h"

void (*Task0A)(void);
void (*Task0B)(void);
void (*Task1A)(void);
void (*Task1B)(void);
void (*Task2A)(void);
void (*Task2B)(void);
void (*Task3A)(void);
void (*Task3B)(void);
void (*Task4A)(void);
void (*Task4B)(void);

void EnableInterrupts(void);
void DisableInterrupts(void);
uint32_t StartCritical(void);
void EndCritical(uint32_t sr);


/** SysTick_Init
 * Initialize Systick interrupt and values 
 * Make sure this is the second to lowest priority
 */
void SysTick_Init(uint32_t period){
	NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
	NVIC_ST_CURRENT_R = 0;      // any write to current clears it
	NVIC_SYS_PRI3_R =(NVIC_SYS_PRI3_R&0x00FFFFFF)|0xD0000000; // priority 6
	NVIC_SYS_PRI3_R =(NVIC_SYS_PRI3_R&0xFF00FFFF)|0x00E00000; // priority 7
}

void Timer0A_Init(void (*task)(void), uint32_t period, uint32_t priority){
	uint32_t sr = StartCritical(); 
	SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
	Task0A = task;
	TIMER0_CTL_R &= ~TIMER_CTL_TAEN;    // 1) disable TIMER0A during setup
	TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;    // 2) configure for 32-bit mode
	TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;   // 3) configure for periodic mode, default down-count settings
	TIMER0_TAILR_R = period - 1;    // 4) reload value
	TIMER0_TAPR_R = 0;            // 5) bus clock resolution
	TIMER0_ICR_R = TIMER_ICR_TATOCINT;    // 6) clear TIMER0A timeout flag
	TIMER0_IMR_R |= TIMER_IMR_TATOIM;    // 7) arm timeout interrupt
	priority &= 0x07;
	NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|(priority << 29); // 8) priority 1
	// interrupts enabled in the main program after all devices initialized
	// vector number 35, interrupt number 19
	NVIC_EN0_R |= 1<<19;           // 9) enable IRQ 19 in NVIC
	TIMER0_CTL_R |= TIMER_CTL_TAEN;    // 10) enable TIMER0A
	EndCritical(sr);
}

void Timer0B_Init(void (*task)(void), uint32_t period, uint32_t priority){
	uint32_t sr = StartCritical(); 
	SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
	Task0B = task;
	TIMER0_CTL_R &= ~TIMER_CTL_TBEN;    // 1) disable TIMER0B during setup
	TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;    // 2) configure for 32-bit mode
	TIMER0_TBMR_R = TIMER_TBMR_TBMR_PERIOD;   // 3) configure for periodic mode, default down-count settings
	TIMER0_TBILR_R = period - 1;    // 4) reload value
	TIMER0_TBPR_R = 0;            // 5) bus clock resolution
	TIMER0_ICR_R = TIMER_ICR_TBTOCINT;    // 6) clear TIMER0B timeout flag
	TIMER0_IMR_R |= TIMER_IMR_TBTOIM;    // 7) arm timeout interrupt
	priority &= 0x07;
	NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFFFF00)|(priority << 5); // 8) priority 1
	// interrupts enabled in the main program after all devices initialized
	// vector number 36, interrupt number 20
	NVIC_EN0_R |= 1<<20;           // 9) enable IRQ 20 in NVIC
	TIMER0_CTL_R |= TIMER_CTL_TBEN;    // 10) enable TIMER0B
	EndCritical(sr);
}

void Timer1A_Init(void (*task)(void), uint32_t period, uint32_t priority){
	uint32_t sr = StartCritical();
	SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
	Task1A = task;
	TIMER1_CTL_R &= ~TIMER_CTL_TAEN;    // 1) disable TIMER1A during setup
	TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;    // 2) configure for 32-bit mode
	TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;   // 3) configure for periodic mode, default down-count settings
	TIMER1_TAILR_R = period-1;    // 4) reload value
	TIMER1_TAPR_R = 0;            // 5) bus clock resolution
	TIMER1_ICR_R = TIMER_ICR_TATOCINT;    // 6) clear TIMER1A timeout flag
	TIMER1_IMR_R |= TIMER_IMR_TATOIM;    // 7) arm timeout interrupt
	priority = (priority & 0x07) << 13; // mask priority (nvic bits 15-13)
	NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)| (priority); // 8) priority 
	
	// interrupts enabled in the main program after all devices initialized
	// vector number 37, interrupt number 21
	NVIC_EN0_R |= 1<<21;           // 9) enable IRQ 21 in NVIC
	TIMER1_CTL_R |= TIMER_CTL_TAEN;    // 10) enable TIMER1A
	EndCritical(sr);
}

void Timer1B_Init(void (*task)(void), uint32_t period, uint32_t priority){
	uint32_t sr = StartCritical();
	SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
	Task1B = task;
	TIMER1_CTL_R &= ~TIMER_CTL_TBEN;    // 1) disable TIMER1B during setup
	TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;    // 2) configure for 32-bit mode
	TIMER1_TBMR_R = TIMER_TBMR_TBMR_PERIOD;   // 3) configure for periodic mode, default down-count settings
	TIMER1_TBILR_R = period-1;    // 4) reload value
	TIMER1_TBPR_R = 0;            // 5) bus clock resolution
	TIMER1_ICR_R = TIMER_ICR_TBTOCINT;    // 6) clear TIMER1B timeout flag
	TIMER1_IMR_R |= TIMER_IMR_TBTOIM;    // 7) arm timeout interrupt
	priority &= 0x07;
	NVIC_PRI5_R = (NVIC_PRI5_R&0xFF00FFFF)| (priority << 21); // 8) priority 
	
	// interrupts enabled in the main program after all devices initialized
	// vector number 38, interrupt number 22
	NVIC_EN0_R |= 1<<22;           // 9) enable IRQ 22 in NVIC
	TIMER1_CTL_R |= TIMER_CTL_TBEN;    // 10) enable TIMER1B
	EndCritical(sr);
}

void Timer2A_Init(void (*task)(void), uint32_t period, uint32_t priority){
	SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate timer2
  Task2A = task;          // user function
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN;    // 1) disable timer2A during setup
  TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;    // 2) configure for 32-bit mode
  TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;   // 3) configure for periodic mode, default down-count settings
  TIMER2_TAILR_R = period-1;    // 4) reload value
  TIMER2_TAPR_R = 0;            // 5) bus clock resolution
  TIMER2_ICR_R = TIMER_ICR_TATOCINT;    // 6) clear timer2A timeout flag
  TIMER2_IMR_R |= TIMER_IMR_TATOIM;    // 7) arm timeout interrupt
	priority &= 0x07;
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|(priority << 29); // 8) priority
// interrupts enabled in the main program after all devices initialized
// vector number 39, interrupt number 23
  NVIC_EN0_R |= 1<<23;           // 9) enable IRQ 23 in NVIC
  TIMER2_CTL_R |= TIMER_CTL_TAEN;    // 10) enable timer2A
}

void Timer2B_Init(void (*task)(void), uint32_t period, uint32_t priority){
	SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate timer2
  Task2B = task;          // user function
  TIMER2_CTL_R &= ~TIMER_CTL_TBEN;    // 1) disable timer2B during setup
  TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;    // 2) configure for 32-bit mode
  TIMER2_TBMR_R = TIMER_TBMR_TBMR_PERIOD;   // 3) configure for periodic mode, default down-count settings
  TIMER2_TBILR_R = period-1;    // 4) reload value
  TIMER2_TBPR_R = 0;            // 5) bus clock resolution
  TIMER2_ICR_R = TIMER_ICR_TBTOCINT;    // 6) clear timer2B timeout flag
  TIMER2_IMR_R |= TIMER_IMR_TBTOIM;    // 7) arm timeout interrupt
	priority &= 0x07;
  NVIC_PRI6_R = (NVIC_PRI6_R&0xFFFFFF00)|(priority << 5); // 8) priority
// interrupts enabled in the main program after all devices initialized
// vector number 40, interrupt number 24
  NVIC_EN0_R |= 1<<24;           // 9) enable IRQ 24 in NVIC
  TIMER2_CTL_R |= TIMER_CTL_TBEN;    // 10) enable timer2B
}

void Timer3A_Init(void (*task)(void), uint32_t period, uint32_t priority){
	uint32_t sr = StartCritical();
	SYSCTL_RCGCTIMER_R |= 0x08;   // 0) activate TIMER3
	Task3A = task;
	TIMER3_CTL_R &= ~TIMER_CTL_TAEN;    // 1) disable TIMER3A during setup
	TIMER3_CFG_R = TIMER_CFG_32_BIT_TIMER;    // 2) configure for 32-bit mode
	TIMER3_TAMR_R = TIMER_TAMR_TAMR_PERIOD;   // 3) configure for periodic mode, default down-count settings
	TIMER3_TAILR_R = period - 1;   // 4) reload value
	TIMER3_TAPR_R = 0;            // 5) bus clock resolution
	TIMER3_ICR_R = TIMER_ICR_TATOCINT;    // 6) clear TIMER3A timeout flag
	TIMER3_IMR_R |= TIMER_IMR_TATOIM;    // 7) arm timeout interrupt
	priority &= 0x07;
	NVIC_PRI8_R = (NVIC_PRI8_R&0x00FFFFFF)|(priority << 29); // 8) priority
	
	#if JITTER
	Timer3_Period = period;
	#endif
	
	// interrupts enabled in the main program after all devices initialized
	// vector number 51, interrupt number 35
	NVIC_EN1_R |= 1<<(35-32);      // 9) enable IRQ 35 in NVIC
	TIMER3_CTL_R |= TIMER_CTL_TAEN;    // 10) enable TIMER3A
	EndCritical(sr);
}

void Timer3B_Init(void (*task)(void), uint32_t period, uint32_t priority){
	uint32_t sr = StartCritical();
	SYSCTL_RCGCTIMER_R |= 0x08;   // 0) activate TIMER3
	Task3B = task;
	TIMER3_CTL_R &= ~TIMER_CTL_TBEN;    // 1) disable TIMER3B during setup
	TIMER3_CFG_R = TIMER_CFG_32_BIT_TIMER;    // 2) configure for 32-bit mode
	TIMER3_TBMR_R = TIMER_TBMR_TBMR_PERIOD;   // 3) configure for periodic mode, default down-count settings
	TIMER3_TBILR_R = period - 1;   // 4) reload value
	TIMER3_TBPR_R = 0;            // 5) bus clock resolution
	TIMER3_ICR_R = TIMER_ICR_TBTOCINT;    // 6) clear TIMER3A timeout flag
	TIMER3_IMR_R |= TIMER_IMR_TBTOIM;    // 7) arm timeout interrupt
	priority &= 0x07;
	NVIC_PRI9_R = (NVIC_PRI9_R&0xFFFFFF00)|(priority << 5); // 8) priority
	
	// interrupts enabled in the main program after all devices initialized
	// vector number 52, interrupt number 36
	NVIC_EN1_R |= 1<<(36-32);      // 9) enable IRQ 35 in NVIC
	TIMER3_CTL_R |= TIMER_CTL_TBEN;    // 10) enable TIMER3B
	EndCritical(sr);
}

void Timer4A_Init(void (*task)(void), uint32_t period, uint32_t priority){
	uint32_t sr = StartCritical();
	SYSCTL_RCGCTIMER_R |= 0x10;   	// 0) activate TIMER4
	Task4A = task;
	TIMER4_CTL_R &= ~TIMER_CTL_TAEN;    	// 1) disable TIMER4A during setup
	TIMER4_CFG_R = TIMER_CFG_32_BIT_TIMER;    	// 2) configure for 32-bit mode
	TIMER4_TAMR_R = TIMER_TAMR_TAMR_PERIOD;   	// 3) configure for periodic mode, default down-count settings
	TIMER4_TAILR_R = period - 1;    	// 4) reload value
	TIMER4_TAPR_R = 0;            	// 5) bus clock resolution
	TIMER4_ICR_R = TIMER_ICR_TATOCINT;    	// 6) clear TIMER4A timeout flag
	TIMER4_IMR_R |= TIMER_IMR_TATOIM;    	// 7) arm timeout interrupt
	priority &= 0x07;
	
	#if JITTER
	Timer4_Period = period;
	#endif
	
	NVIC_PRI17_R = (NVIC_PRI17_R&0xFF00FFFF);
	NVIC_PRI17_R = (NVIC_PRI17_R | (priority << 21)); // 8) priority
	// interrupts enabled in the main program after all devices initialized
	// vector number 86, interrupt number 70
	NVIC_EN2_R = 1<<(70-64);      // 9) enable IRQ 70 in NVIC 
	TIMER4_CTL_R |= TIMER_CTL_TAEN;    // 10) enable TIMER4A
	EndCritical(sr);
}

void Timer4B_Init(void (*task)(void), uint32_t period, uint32_t priority){
	uint32_t sr = StartCritical();
	SYSCTL_RCGCTIMER_R |= 0x10;   	// 0) activate TIMER4
	Task4B = task;
	TIMER4_CTL_R &= ~TIMER_CTL_TBEN;    	// 1) disable TIMER4B during setup
	TIMER4_CFG_R = TIMER_CFG_32_BIT_TIMER;    	// 2) configure for 32-bit mode
	TIMER4_TBMR_R = TIMER_TBMR_TBMR_PERIOD;   	// 3) configure for periodic mode, default down-count settings
	TIMER4_TBILR_R = period - 1;    	// 4) reload value
	TIMER4_TBPR_R = 0;            	// 5) bus clock resolution
	TIMER4_ICR_R = TIMER_ICR_TBTOCINT;    	// 6) clear TIMER4B timeout flag
	TIMER4_IMR_R |= TIMER_IMR_TBTOIM;    	// 7) arm timeout interrupt
	priority &= 0x07;
	NVIC_PRI17_R = (NVIC_PRI17_R&0x00FFFFFF);
	NVIC_PRI17_R = (NVIC_PRI17_R | (priority << 29)); // 8) priority
	// interrupts enabled in the main program after all devices initialized
	// vector number 87, interrupt number 71
	NVIC_EN2_R = 1<<(71-64);      // 9) enable IRQ 71 in NVIC 
	TIMER4_CTL_R |= TIMER_CTL_TBEN;    // 10) enable TIMER4B
	EndCritical(sr);
}





void Timer0A_Handler(void){
	TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
	Task0A();
}

void Timer0B_Handler(void){
	TIMER0_ICR_R = TIMER_ICR_TBTOCINT;// acknowledge timer0B timeout
	Task0B();
}

void Timer1A_Handler(void){
	TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer1A timeout
	Task1A();
}

void Timer1B_Handler(void){
	TIMER1_ICR_R = TIMER_ICR_TBTOCINT;// acknowledge timer1B timeout
	Task1B();
}

void Timer2A_Handler(void){
	TIMER2_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer2A timeout
	Task2A();
}

void Timer2B_Handler(void){
	TIMER2_ICR_R = TIMER_ICR_TBTOCINT;// acknowledge timer2B timeout
	Task2B();
}

void Timer3A_Handler(void){
	TIMER3_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer3A timeout
	Task3A();
}

void Timer3B_Handler(void){
	TIMER3_ICR_R = TIMER_ICR_TBTOCINT;// acknowledge timer3B timeout
	Task3B();
}

void Timer4A_Handler(void){
	TIMER4_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer4A timeout
	Task4A();
}

void Timer4B_Handler(void){
	TIMER4_ICR_R = TIMER_ICR_TBTOCINT;// acknowledge timer4B timeout
	Task4B();
}

