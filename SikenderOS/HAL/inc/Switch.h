/** @file Switch.c
 * @brief Board Switch setup for TM4c123
 * @author Sijin Woo (https://github.com/SijWoo)
 */


#ifndef SWITCH_H
#define SWITCH_H


#include "cpu_vars.h"


void SW1_Init(void (*task)(void), INT32U priority);

void SW2_Init(void (*task)(void), INT32U priority);

void SW1_Debounce(void);

void SW2_Debounce(void);

#endif

