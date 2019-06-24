/** @file main.c
 * @brief Example program using SikendeRTOS features
 * @date 2/9/2019
 */

#include "OS.h"
#include "UART0.h"
#include "Interpreter.h"
#include <stdint.h>
#include "cpu.h"


#define GREEN_LED			PF3
#define GREEN_BLINK 		0x8

#define BLUE_LED			PF2
#define BLUE_BLINK			0x4

#define RED_LED				PF1
#define RED_BLINK			0x2


int ThreadsCreated = 0; // number of threads created by main
Sema4Type Mutex_REDLED;


/**
 * @brief Blink Green LED
**/
void Flash_Green(void)
{
	while(1)
	{
	GREEN_LED ^= GREEN_BLINK;
	OS_Suspend();

	}
}


/** 
 * @brief Blink Blue LED
**/
void Flash_Blue(void)
{
	while(1)
	{
	BLUE_LED ^= BLUE_BLINK;
	OS_Sleep(3);
	}
}


/**
 * @brief Blink Red LED
**/
void Flash_Red(void)
{
	while(1)
	{
		OS_bWait(&Mutex_REDLED);
		RED_LED ^= RED_BLINK;
		OS_bSignal(&Mutex_REDLED);
	}
}


/**
 * @brief Blink Red LED
**/
void Flash_Red2(void)
{
	while(1)
	{
		OS_ASM_Wait(&Mutex_REDLED);
		RED_LED ^= RED_BLINK;
		OS_ASM_Signal(&Mutex_REDLED);
	}
}


/**
 * @brief Run interpreter using UART0 (in USB debugger)
**/
void WrapInterpreter(void)
{
	while(1){
		Interpreter();
	}
}

/**
 * @brief Dummy Thread, Prevent OS crash
**/
void DummyThread(void){
	volatile int dummyCount = 0;
	while(1){
		dummyCount++;
	}
}


/** 
 * @brief Example usecase of RTOS 
**/
int main(void)
{    
	LED_Init();
	OS_Init();           
	
	//********initialize communication channels
	OS_MailBox_Init();
	OS_Fifo_Init();
	
	ThreadsCreated += OS_AddThread(&WrapInterpreter, TIME_1MS * 16, 0); //60 Hz UART interface
	
	// create initial foreground threads
	if(OS_AddThread(&WrapInterpreter,128,2))
		ThreadsCreated++;
	
	// create initial foreground threads
	if(OS_AddThread(&DummyThread,128,2))
		ThreadsCreated++;
	
	// create initial foreground threads
	if(OS_AddThread(&Flash_Green,128,1))
		ThreadsCreated++;
	
	// create initial foreground threads
	if(OS_AddThread(&Flash_Blue,128,2))
		ThreadsCreated++;

	// create initial foreground threads
	if(OS_AddThread(&Flash_Red,128,2))
		ThreadsCreated++;
	
	OS_InitSemaphore(&Mutex_REDLED, 1);
	OS_Launch(TIME_2MS);
	
	// this should never run
	while(1){}
	return 0;            // this never executes
}


/**
 * @brief HArdfault Handler Sets LEDS to white to indicate fault, assumes LEDS already initialized 
**/
void HardFault_Handler(void){
	PF1 = 0x02;
	PF2 = 0x04;
	PF3 = 0x08;
	while(1);
}
