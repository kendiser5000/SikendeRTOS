/** @file main.c
 * @brief Example program using SikendeRTOS features
 * @date 2/9/2019
 */

#include "OS.h"
#include "tm4c123gh6pm.h"
#include "UART0.h"
#include "Interpreter.h"
#include <stdint.h>
#include <stdbool.h>


// LED port register locations
#define PF1  (*((volatile uint32_t *)0x40025008))
#define PF2  (*((volatile uint32_t *)0x40025010))
#define PF3  (*((volatile uint32_t *)0x40025020))

#define GREEN_LED	PF3
#define GREEN_BLINK 0x8

#define BLUE_LED	PF2
#define BLUE_BLINK	0x4

#define RED_LED		PF1
#define RED_BLINK	0x2


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode


/** 
 * @brief Initialize ports for blinking LEDs on TM4C123/TM4C129 Development Board 
**/
void LED_Init(void)
{   
	SYSCTL_RCGCGPIO_R |= 0x20;     				// Enable Clock
	while((SYSCTL_PRGPIO_R & 0x20)!=0x20){};	// wait for clock to be enabled     
	GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;// 2a) unlock GPIO Port F Commit Register
	GPIO_PORTF_CR_R = 0x0F;        // 2b) enable commit for PF0-PF3    
	GPIO_PORTF_AMSEL_R &= ~0x0F;   // 3) disable analog functionality on PF0-PF3     
	GPIO_PORTF_PCTL_R &= ~0x000FFFF;// 4) configure PF0-PF3 as GPIO
	GPIO_PORTF_DIR_R = 0x0F;       // 5) make PF0-3 output                       
	GPIO_PORTF_AFSEL_R &= ~0x0F;        // 6) disable alt funct on PF0-PF3
	GPIO_PORTF_DEN_R = 0x0F;       // 7) enable digital I/O on PF0-PF3
}

/**
 * @brief Blink Green LED
**/
void Flash_Green(void)
{
	GREEN_LED ^= GREEN_BLINK;
	OS_Suspend();
}

/**
 * @brief Blink Red LED
**/
void Flash_Red(void)
{
	RED_LED ^= RED_BLINK;
}

/** 
 * @brief Blink Blue LED
**/
void Flash_Blue(void)
{
	BLUE_LED ^= BLUE_BLINK;
}

/**
 * @brief Run interpreter using UART0 (in USB debugger)
**/
void WrapInterpreter(void){
	while(1){
		Interpreter();
	}
}




int NumCreated = 0; // number of threads created by main
/** 
 * @brief Example usecase of RTOS 
**/
int main(void)
{    
	LED_Init();
	OS_Init();           
	
	//********initialize communication channels
	OS_MailBox_Init();
	OS_Fifo_Init(128);    // ***note*** 4 is not big enough*****//128
	
	
	OS_AddThread(&WrapInterpreter, 80000000/20, 0); //20 Hz data Collection
	
	// create initial foreground threads
	if(OS_AddThread(&WrapInterpreter,128,2))
		NumCreated++;


	OS_Launch(TIME_2MS); // doesn't return, interrupts enabled in here
	
	// this should never run
	while(1){}
	return 0;            // this never executes
}


void HardFault_Handler(void){
	PF1 = 0x02;
	PF2 = 0x04;
	PF3 = 0x08;
	while(1);
}
