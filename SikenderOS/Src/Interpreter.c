/** @file Interpreter.c
 * @brief Runs on TM4C123
 * UART Command Line Interface
 * @date 2/9/2019
 * @author Sikender Ashraf and Sijin Woo
 */
 
#include <stdint.h>
#include "UART0.h"
#include "tm4c123gh6pm.h"
#include "OS.h"



/** newLine
 *	Create new line on UART interface
 * @date 2/04/2019
*/
void newLine(void){
	OutCRLF();
}



/** Interpreter_RealMain
 * Interpreter module for lab 2 realmain
 * Shows datalost, PID calculations finished, digital filter calculators finished
 * @parameter DataLost
 * @parameter PIDWork
 * @parameter FilterWork
 * @date 2/04/2019
*/
void Interpreter(){
	while(1)
	{
		newLine();
	}
}



