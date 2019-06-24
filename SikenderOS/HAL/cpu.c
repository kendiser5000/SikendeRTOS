/**
* @file cpu.c
* @brief Contains CPU Initializations and Register definition (this is example bc project uses legacy register map)
*			Should contain all wrapper/abstracted functions for peripheral setup and use
* @date 6/19/2018
*/

#include "cpu.h"


void CLOCK_SystemEnable(void)
{
	PLL_Init(Bus80MHz);         // set processor clock to 50 MHz

}



/** 
 * @brief Initialize ports for blinking LEDs on TM4C123/TM4C129 Development Board 
 * 			example function
**/
void LED_Init(void)
{   
	SYSCTL_RCGCGPIO_R |= CLOCK_MASK_PORTF;     				// Enable Clock
	while((SYSCTL_PRGPIO_R & CLOCK_MASK_PORTF) != CLOCK_MASK_PORTF){};	// wait for clock to be enabled     
	GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;// 2a) unlock GPIO Port F Commit Register
	GPIO_PORTF_CR_R = LEDS;        // 2b) enable commit for PF0-PF3    
	GPIO_PORTF_AMSEL_R &= ~LEDS;   // 3) disable analog functionality on PF0-PF3     
	GPIO_PORTF_PCTL_R &= ~0x000FFFF;// 4) configure PF0-PF3 as GPIO
	GPIO_PORTF_DIR_R = LEDS;       // 5) make PF0-3 output                       
	GPIO_PORTF_AFSEL_R &= ~LEDS;        // 6) disable alt funct on PF0-PF3
	GPIO_PORTF_DEN_R = LEDS;       // 7) enable digital I/O on PF0-PF3
}

