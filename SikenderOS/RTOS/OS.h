/**
* @file OS.h
* @brief Functions for OS
* @date 2/04/2019
*/

 
#ifndef __OS_H
#define __OS_H  

#include <stdint.h>
#include "OSConfig.h"


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
 * @brief Wait on semaphore, binary
 * @parameter semaPt pointer to semaphore
*/
void OS_bWait(Sema4Type *semaPt); 



/** OS_bSignal
 * @brief Signal semaphore to be free, set to 1
*/
void OS_bSignal(Sema4Type *semaPt); 


/** OS_ASM_Signal
 * @brief Spinlock semaphore signal using ARM exclusion
*/
void OS_ASM_Signal(Sema4Type *semaPt);



/** OS_ASM_Signal
 * @brief Spinlock semaphore wait using ARM exclusion
*/
void OS_ASM_Wait(Sema4Type *semaPt);


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
unsigned long OS_IdThread(void);


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
 * Initialize OS Fifo, size must be power of 2, inits semaphores
*/
void OS_Fifo_Init(void);



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
 * @brief Initialize mailbox for OS
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
