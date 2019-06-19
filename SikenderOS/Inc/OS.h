/**
* @file OS.h
* @brief Functions for OS
* @date 2/04/2019
*/

 
#ifndef __OS_H
#define __OS_H  1

#include <stdint.h>
#include "PLL.h"
#include "tm4c123gh6pm.h"
#include "UART0.h"

// edit these depending on your clock  
#define BUS_CLK		80000000		// 80 Mhz
#define TIME_1MS    BUS_CLK/1000
#define TIME_2MS    (2*TIME_1MS)  
#define TIME_500US  (TIME_1MS/2)  
#define TIME_250US  (TIME_1MS/5)  
#define PERIOD TIME_500US


//***************** OS CONFIGURATION **********************/
/** NUMTHREADS
 * @brief max number of threads
*/
#define NUMTHREADS  10        // maximum number of threads

/** STACKSIZE
 * @brief max size of stack
*/
#define STACKSIZE   256       // number of 32-bit words in stack

/** PRIORITYLEVELS
 * @brief number of priorities
*/
#define PRIORITYLEVELS 8       // 0-7, follow arm protocol

/**
 * OS FIFO SIZE
 * @brief Size of OS FIFO in 32 bit words
 */
#define FIFO_SIZE 256

/**
 * OS Scheduler Mode
 * @brief Set mode of scheduler,
 * 			1: Priority Scheduler (Blocking semaphores)
 *			0: Round Robin Scheduler (Spin-Lock semaphores)
 */
#define SCHEDULER_MODE 1
//************* END CONFIGURATION ************************

/** Semaphore 
 * Semaphore structure, add FiFo later?
*/
struct  Sema4{
	long Value;   // 0 is free, >0 not free I think       
	struct Tcb* blockThreads;
};
typedef struct Sema4 Sema4Type;



/** Mailbox 
 * Contains data and semaphores for mailbox 
 * interthread communication
*/
struct MailBox{
	Sema4Type Empty;
	Sema4Type Full;
	uint32_t data;
};
typedef struct MailBox MailBoxType;




/** Dummy function
*/
void dummy(void);


/** OS_Init
 * Initializes operating system, disables interrupts until OS_Launch
 * Initializes all I/O and interthread communication
*/
void OS_Init(void); 


/** OS_InitSemaphore
 * Initializes semaphore to default value sent
 * @parameter semaPt pointer to semaphore
 * @parameter value value of semaphore
*/
void OS_InitSemaphore(Sema4Type *semaPt, long value); 


/** OS_Wait
 * Wait on semaphore, if busy yield
 * @parameter semaPt pointer to semaphore
*/
void OS_Wait(Sema4Type *semaPt); 


/** OS_Signal
 * Clear Semaphore, Increment
*/
void OS_Signal(Sema4Type *semaPt); 


/** OS_bWait
 * Wait on semaphore, binary
 * @parameter semaPt pointer to semaphore
*/
void OS_bWait(Sema4Type *semaPt); 



/** OS_bSignal
 * Signal semaphore to be free, set to 1
*/
void OS_bSignal(Sema4Type *semaPt); 



/** OS_AddThread
 * Add new thread to OS, Linked List style
 * @parameter task task to run for thread
 * @parameter stackSize size of stack
 * @parameter priority priority of thread
 * @return success: 1, fail: 0
*/
int OS_AddThread(void(*task)(void), unsigned long stackSize, unsigned long priority);

 
/** OS_Id
 * return ID of current thread
 * @return ID of thread
*/
unsigned long OS_Id(void);


/** OS_AddPeriodicThread
 * Add new thread to OS that runs periodically, uses Timer4
 * @parameter task task to run for thread
 * @parameter stackSize size of stack
 * @parameter priority priority of thread
 * @return success: 1, fail: 0
*/
int OS_AddPeriodicThread(void(*task)(void), unsigned long period, unsigned long priority);



/** OS_AddSW1Task
 * Add thread to button PF4
 * @parameter task task to run for thread
 * @parameter priority priority of thread
 * @return success: 1, fail: 0
*/
int OS_AddSW1Task(void(*task)(void), unsigned long priority);


/** OS_AddSW1Task
 * Add thread to button PF0
 * @parameter task task to run for thread
 * @parameter priority priority of thread
 * @return success: 1, fail: 0
*/
int OS_AddSW2Task(void(*task)(void), unsigned long priority);


/** OS_Sleep
 * Put current thread to sleep then yield
 * @parameter sleepTime amount of time to sleep (ms)
*/
void OS_Sleep(unsigned long sleepTime); 


/** OS_Kill
 * kills current thread
*/
void OS_Kill(void); 


/** OS_Suspend
 * Suspends current thread
*/
void OS_Suspend(void);
 
// ******** OS_Fifo_Init ************
// Initialize the Fifo to be empty
// Inputs: size
// Outputs: none 
// In Lab 2, you can ignore the size field
// In Lab 3, you should implement the user-defined fifo size
// In Lab 3, you can put whatever restrictions you want on size
//    e.g., 4 to 64 elements
//    e.g., must be a power of 2,4,8,16,32,64,128

/** OS_Fifo_Init
 * Initialize OS Fifo, must be power of 2, inits semaphores
 * @parameter size size of the Fifo
*/
void OS_Fifo_Init(unsigned long size);



/** OS_Fifo_Put
 * Put data to Fifo
 * @parameter data data to put in Fifo
 * @return success: 1, fail: 0
*/
int OS_Fifo_Put(unsigned long data);  



/** OS_Fifo_Get
 * Get data from thread
 * @return data
*/
unsigned long OS_Fifo_Get(void);



/** OS_Fifo_Size
 * Get number of items in FiFo
 * @return size of Fifo
*/
long OS_Fifo_Size(void);

// ******** OS_MailBox_Init ************
// Initialize communication channel
// Inputs:  none
// Outputs: none

/** OS_MailBox_Init
 * Initialize Fifo
*/
void OS_MailBox_Init(void);


/** OS_MailBox_Send
 * Puts data in mailbox
 * @parameter data
*/
void OS_MailBox_Send(unsigned long data);


/** OS_MailBox_Recv
 * Get data from Mailbox
 * @return data
*/
unsigned long OS_MailBox_Recv(void);


/** OS_Time
 * Returns time in 12.5ns
 * @parameter time
*/
unsigned long OS_Time(void);


/** OS_TimeDifference
 * Return difference between time
 * @parameter start start time
 * @parameter stop stop time
 * @return time difference
*/
unsigned long OS_TimeDifference(unsigned long start, unsigned long stop);


/** OS_ClearMsTime
 * clear ms time counter
*/
void OS_ClearMsTime(void);


/** OS_MsTime
 * return time (ms)
 * @return time in ms
*/
unsigned long OS_MsTime(void);


/** OS_Launch
 * Start OS, setup SysTick 
 * @parameter theTimeSlice Time between context switches
*/
void OS_Launch(unsigned long theTimeSlice);


/** OS_AddThreads
 * Add 3 threads
 * @parameter task0
 * @parameter task1
 * @parameter task2
 * @return success: 1, fail: 0
*/
int OS_AddThreads(void(*task0)(void), void(*task1)(void), void(*task2)(void));


/** OS_ClearPeriodicTime
 * The following function clears the period time, resets counter
 * @returns 1 if success
 */
void OS_ClearPeriodicTime(void);

/** OS_ReadPeriodicTime
 * The following function reads counter
 * @channelNum specifies channel to initialize
 * @returns 1 if success
 */ 
uint32_t OS_ReadPeriodicTime(void);


/** Jitter
 *  Output Jitter Value to screen
*/
void Jitter(void);


#endif
