/**
* @file startup.h
* @brief Contains Enable/Disable Interrupts, Start/EndCritical from startup file
* 			Used to control global ISR flag
*/
#ifndef __startup_H__
#define __startup_H__


#include "cpu.h"

/**
* @brief Enable Global Interrupts (ISR = 0)
* @date 6/19/2018
*/
void EnableInterrupts(void);


/**
* @brief Disable Global Interrupts (ISR = 1)
*/
void DisableInterrupts(void);


/**
* @brief Start Criticat Sections, Disable Global Interrupts
* @return value of current PSR
*/
INT32U StartCritical(void);


/**
* @brief End of critical section (ISR = prev ISR)
* @parameter sr previously saved PSR
* @author Valvano
*/
void EndCritical(INT32U sr);

/**
* @brief Low power mode, wake up on interrupt
*/
void WaitForInterrupt(void);  // low power mode



#endif // __startup_H__

