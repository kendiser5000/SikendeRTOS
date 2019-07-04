/**
* @file OS.h
* @brief Functions for OS
*/
#ifndef __OS_H
#define __OS_H  


#include "OSConfig.h"
#include "cpu_vars.h"


/** Semaphore 
 * Semaphore structure
*/
struct  Sema4{
	INT32 Value;   // 0 is free, >0 not free I think       
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
	INT32U data;
};
typedef struct MailBox MailBoxType;


/** OS_Init
 * Initializes operating system, disables interrupts until OS_Launch
 * Initializes all I/O and interthread communication
*/
void OS_Init(void); 


/** OS_InitSemaphore
 * Initializes semaphore to default value sent
 * @param semaPt pointer to semaphore
 * @param value value of semaphore
*/
void OS_InitSemaphore(Sema4Type *semaPt, INT32 value); 


/** OS_Wait
 * Wait on semaphore, if busy yield
 * @param semaPt pointer to semaphore
*/
void OS_Wait(Sema4Type *semaPt); 


/** OS_Signal
 * Clear Semaphore, Increment
*/
void OS_Signal(Sema4Type *semaPt); 


/** OS_bWait
 * @brief Wait on semaphore, binary
 * @param semaPt pointer to semaphore
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
 * @param task task to run for thread
 * @param stackSize size of stack
 * @param priority priority of thread
 * @return success: 1, fail: 0
*/
INT8 OS_AddThread(void(*task)(void), INT32U priority);

 
/** OS_Id
 * return ID of current thread
 * @return ID of thread
*/
INT32U OS_IdThread(void);


/** OS_AddPeriodicThread
 * Add new thread to OS that runs periodically, uses Timer4
 * @param task task to run for thread
 * @param stackSize size of stack
 * @param priority priority of thread
 * @return success: 1, fail: 0
*/
INT8 OS_AddPeriodicThread(void(*task)(void), INT32U period, INT32U priority);



/** OS_AddSW1Task
 * Add thread to button PF4
 * @param task task to run for thread
 * @param priority priority of thread
 * @return success: 1, fail: 0
*/
INT8 OS_AddSW1Task(void(*task)(void), INT32U priority);


/** OS_AddSW1Task
 * Add thread to button PF0
 * @param task task to run for thread
 * @param priority priority of thread
 * @return success: 1, fail: 0
*/
INT8 OS_AddSW2Task(void(*task)(void), INT32U priority);


/** OS_Sleep
 * Put current thread to sleep then yield
 * @param sleepTime amount of time to sleep (ms)
*/
void OS_Sleep(INT32U sleepTime); 



/** OS_Kill
 * kills current thread
*/
void OS_Kill(void); 



/** OS_Suspend
 * Suspends current thread
*/
void OS_Suspend(void);
 


/** OS_Fifo_Init
 * Initialize OS Fifo, size must be power of 2, inits semaphores
*/
void OS_Fifo_Init(void);



/** OS_Fifo_Put
 * Put data to Fifo
 * @param data data to put in Fifo
 * @return success: 1, fail: 0
*/
INT8 OS_Fifo_Put(FIFO_t data);  



/** OS_Fifo_Get
 * Get data from thread
 * @return data
*/
FIFO_t OS_Fifo_Get(void);



/** OS_Fifo_Size
 * Get number of items in FiFo
 * @return size of Fifo
*/
INT32U OS_Fifo_Size(void);


/** OS_MailBox_Init
 * @brief Initialize mailbox for OS
*/
void OS_MailBox_Init(void);


/** OS_MailBox_Send
 * Puts data in mailbox
 * @param data
*/
void OS_MailBox_Send(INT32U data);


/** OS_MailBox_Recv
 * Get data from Mailbox
 * @return data
*/
INT32U OS_MailBox_Recv(void);


/** OS_Time
 * Returns time in 12.5ns
*/
INT32U OS_Time(void);


/** OS_TimeDifference
 * Return difference between time
 * @param start start time
 * @param stop stop time
 * @return time difference
*/
INT32U OS_TimeDifference(INT32U start, INT32U stop);


/** OS_ClearMsTime
 * clear ms time counter
*/
void OS_ClearMsTime(void);


/** OS_ReadMsTime
 * return time (ms)
 * @return time in ms
*/
INT32U OS_ReadMsTime(void);


/** OS_Launch
 * Start OS, setup SysTick 
 * @param theTimeSlice Time between context switches
*/
void OS_Launch(INT32U theTimeSlice);


/** OS_ClearMsTime
 * The following function clears the period time, resets counter
 * @returns 1 if success
 */
void OS_ClearMsTime(void);





#endif // _OS_H_
