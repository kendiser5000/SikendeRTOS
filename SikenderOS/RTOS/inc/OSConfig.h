/**
* @file OSConfig.h
* @brief Configuration setup for OS
* @date 2/04/2019
*/
#ifndef __OSCONFIG_H
#define __OSCONFIG_H  

#include "cpu_vars.h"


// edit these depending on your clock  
#define TIME_1MS    BUS_CLK/1000
#define TIME_2MS    (2*TIME_1MS)  
#define TIME_500US  (TIME_1MS/2)  
#define TIME_250US  (TIME_1MS/5)  
#define PERIOD TIME_500US


//***************** OS CONFIGURATION **********************/
/** NUMTHREADS
 * @brief max number of threads
*/
#define NUMTHREADS  10        

/** STACKSIZE
 * @brief max size of stack
*/
#define STACKSIZE   256       

/** PRIORITYLEVELS
 * @brief number of priorities
*/
#define PRIORITYLEVELS 8       // 0-7, follows ARM interrupt protocol

/**
 * OS FIFO SIZE
 * @brief Size of OS FIFO in 32 bit words
 */
#define FIFO_SIZE 256

typedef INT32 FIFO_t; 


/**
 * OS Scheduler Mode
 * @brief Set mode of scheduler,
 * 			1: Priority Scheduler (Blocking semaphores)
 *			0: Round Robin Scheduler (Spin-Lock semaphores)
 */
#define SCHEDULER_MODE 1



#endif //_OSConfig_H

