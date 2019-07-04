/** @file Interpreter.h
 * @brief Runs on TM4C123
 * Command line interface
 * @date 2/9/2019
 * @author Sikender Ashraf and Sijin Woo
 */
 
 
#ifndef __Interpreter_H__
#define __Interpreter_H__

#include "tm4c123gh6pm.h"
#include "cpu_vars.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


#define MAXSAMPLES 1000
#define MAX_WORDS		20
#define MAX_CHARS_PER_WORD		30


/** Interpreter_Start
 * The following function starts Interpreter with UART
 */
void Interpreter(void);

#endif	// __Interpreter_H_
