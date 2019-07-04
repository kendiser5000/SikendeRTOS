/** @file Timer.c
 * @brief Board Switch Setup for TM4c123
 * @author Sijin Woo (https://github.com/SijWoo)
 */

#include "tm4c123gh6pm.h"
#include "OS.h"
#include "Switch.h"


#define SW1		0x10
#define SW2		0x01

void (*SW1_Task)(void);
void (*SW2_Task)(void);

uint32_t SW1_Priority;
uint32_t SW2_Priority;

uint32_t SW1_LastState;
uint32_t SW2_LastState;

void SW1_Init(void (*task)(void), uint32_t priority){
	SYSCTL_RCGCGPIO_R |= 0x00000020; 			// (a) activate clock for port F
	SW1_Task = task;
	while((SYSCTL_PRGPIO_R & 0x00000020) == 0){};
	GPIO_PORTF_CR_R = SW1;           	// allow changes to PF4, Needed?
	GPIO_PORTF_DIR_R &= ~SW1;    		// make PF4 button
	GPIO_PORTF_PUR_R |= SW1;				// Pull up for Button
	GPIO_PORTF_AFSEL_R &= ~SW1;  		// disable alt funct on PF4
	GPIO_PORTF_DEN_R |= SW1;     		// enable digital I/O on PF4   
	GPIO_PORTF_PCTL_R &= ~0x000F0000; 			// configure PF4 as GPIO
	GPIO_PORTF_AMSEL_R = 0;       				// disable analog functionality on PF
	GPIO_PORTF_IS_R &= ~SW1;     		// PF4 is edge-sensitive
	GPIO_PORTF_IBE_R &= ~SW1;     		// Not double edge
	GPIO_PORTF_IEV_R &= ~SW1;     		// PF4 only falling edge
	GPIO_PORTF_ICR_R = SW1;      		// arm interrupt?
	GPIO_PORTF_IM_R |= SW1;      		// arm interrupt on PF4
	SW1_Priority = priority;
	priority = (priority & 0x07) << 21;			// set NVIC priority bit (21-23)
	SW1_LastState = SW1;
	NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF); 	
	NVIC_PRI7_R = (NVIC_PRI7_R | priority); 	
	
	NVIC_EN0_R = 0x40000000;      				// enable interrupt 30 in NVIC 	
}

void SW2_Init(void (*task)(void), uint32_t priority){
	SYSCTL_RCGCGPIO_R |= 0x00000020; 			// activate clock for port F
	SW2_Task = task;
	while((SYSCTL_PRGPIO_R & 0x00000020) == 0){};
	GPIO_PORTF_LOCK_R = 0x4C4F434B; 			// unlock GPIO Port F, just like Sijin unlocks my heart <3
	GPIO_PORTF_CR_R = 0x1F;
	GPIO_PORTF_DIR_R &= ~SW2;    		// PF0 is button
	GPIO_PORTF_PUR_R |= SW2;     		// enable weak pull-up 
	GPIO_PORTF_AFSEL_R &= SW2;  		// disable alt funct 
	GPIO_PORTF_DEN_R |= SW2;     		// enable digital I/O   
	GPIO_PORTF_PCTL_R &= ~0x0000000F; 			// configure PF0 as GPIO
	GPIO_PORTF_AMSEL_R = 0;       				// disable analog functionality
	GPIO_PORTF_IS_R &= ~SW2;     		// Edge-sensitive
	GPIO_PORTF_IBE_R &= ~SW2;     		// not double edge sensitve
	GPIO_PORTF_IEV_R &= ~SW2;     		// falling edge sensitve
	GPIO_PORTF_ICR_R = SW2;      		// clear flag
	GPIO_PORTF_IM_R |= SW2;      		// arm interrupt

	SW2_Priority = priority;
	priority = (priority & 0x07) << 21;						// NVIC priority bit (21-23)

	SW2_LastState = SW2;
		
	NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF); 				
	NVIC_PRI7_R = (NVIC_PRI7_R | priority); 
	NVIC_EN0_R = 0x40000000;      							// enable interrupt 30 in NVIC 	
	
}

void SW1_Debounce(void){
	OS_Sleep(15);
	SW1_LastState = SW1;
	GPIO_PORTF_ICR_R = SW1;		// clear flag
	GPIO_PORTF_IM_R |= SW1;		// arm
	OS_Kill();
}

void SW2_Debounce(void){
	OS_Sleep(15);
	SW2_LastState = SW2;
	GPIO_PORTF_ICR_R = SW2;		// clear flag
	GPIO_PORTF_IM_R |= SW2;		// arm
	OS_Kill();
}

void GPIOPortF_Handler(void){
	// sw1 pressed
	if(GPIO_PORTF_RIS_R & SW1){    
		if (SW1_LastState == SW1){
			(*SW1_Task)();
		} 
		int32_t status = OS_AddThread(&SW1_Debounce, SW1_Priority);
		GPIO_PORTF_IM_R &= ~SW1;    // disarm
		GPIO_PORTF_ICR_R = SW1;      // clear flag
		// cant make thread, probable too many threads, chill out my guy (@TA)
		if(status == 0){ 
			GPIO_PORTF_ICR_R = SW1;      // clear flag
			GPIO_PORTF_IM_R |= SW1;      
		}
	}
	
	// sw2 pressed, cant do else in case both pressed
	if(GPIO_PORTF_RIS_R & SW2){		// SW2 pressed
		if (SW2_LastState == SW2){
			(*SW2_Task)();
		}
		int32_t status = OS_AddThread(&SW2_Debounce, SW2_Priority);
		GPIO_PORTF_IM_R &= ~SW2;     // disarm interrupt
		GPIO_PORTF_ICR_R = SW2;      // clear flag
		// cant make thread, probable too many threads, chill out my guy (@TA)
		if(status == 0){ 
			GPIO_PORTF_ICR_R = SW2;      // clear flag
			GPIO_PORTF_IM_R |= SW2;      
		}
	}
}


