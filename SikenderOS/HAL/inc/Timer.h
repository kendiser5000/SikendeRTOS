/** @file Timer.h
 * @brief Periodic Timer setup for TM4c123
 * @author Sijin Woo (https://github.com/SijWoo)
 */


#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void SysTick_Init(uint32_t period);

void Timer0A_Init(void (*task)(void), uint32_t period, uint32_t priority);

void Timer0B_Init(void (*task)(void), uint32_t period, uint32_t priority);

void Timer1A_Init(void (*task)(void), uint32_t period, uint32_t priority);

void Timer1B_Init(void (*task)(void), uint32_t period, uint32_t priority);

void Timer2A_Init(void (*task)(void), uint32_t period, uint32_t priority);

void Timer2B_Init(void (*task)(void), uint32_t period, uint32_t priority);

void Timer3A_Init(void (*task)(void), uint32_t period, uint32_t priority);

void Timer3B_Init(void (*task)(void), uint32_t period, uint32_t priority);

void Timer4A_Init(void (*task)(void), uint32_t period, uint32_t priority);

void Timer4B_Init(void (*task)(void), uint32_t period, uint32_t priority);

#endif

