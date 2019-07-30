/**
* @file OS.c
* @brief Contains functions to run OS
* 
*/

#include "cpu.h"
#include "startup.h"
#include "Timer.h"
#include "Switch.h"
#include "OS.h"



// OS ASM functions
void StartOS(void);
void OS_EnableInterrupts(void);
void OS_DisableInterrupts(void);


/*! @var INT32U Stacks
    @brief Contains all the stacks for each thread
*/
INT32U Stacks[NUMTHREADS][STACKSIZE];


/*! @var INT8U numOfThreads
    @brief number of threads running
*/
static INT32U NumOfThreads = 0;


/*! @var uint32_T OS_SystemTime
    @brief time OS has been running in ms
*/
INT64U OS_SystemTime = 0;

/*! @var uint32_T OS_SystemTimeMS
    @brief time OS has been running in ms
*/
INT64U OS_SystemTimeMS;


/** @var Thread Control Block
 * @brief Structure for each thread, helps with context switching, Now Doubly linked List
*/
struct Tcb{
	/*@{*/
	INT32U *sp;			/**< pointer to stack (valid for threads not running */
	// double LL
	struct Tcb* next;		/**< Pointer to next TCB */
	struct Tcb* prev;
	// basic properties
	INT32 id;					/**< ID number of thread, negative if unused */
	INT32 status;				/**< status of thread: -1: unused, 1: used */
	INT8U priority;		/**< priority of thread, 0-5 */
	INT32U sleepState;		/**< sleep state */
	// Lab 3 blocking threads
	Sema4Type* sema4Blocked;	/**< blocked state */
	struct Tcb* nextBlocked;
	struct Tcb* nextPriority;
	/*@}*/
};
typedef struct Tcb tcbType;
static tcbType tcbs[NUMTHREADS];

/*! @var tcbType *RunPt
    @brief Contains currently running thread 
*/
tcbType *RunPt;

/*! @var tcbType *NextRunPt
    @brief Contains next thread to run
*/
tcbType *NextRunPt;

/*! @var FIFO_t OS_FIFO
    @brief Contains next thread to run
*/
FIFO_t OS_FIFO[FIFO_SIZE];
INT32U Get_Idx = 0, Put_Idx = 0;
Sema4Type SemaFIFO;


//************* PRIORITY SCHEDULING GLOBALS AND ARRAYS**************************************************************************
//Value > 0, else 0 if empty, change upon OS_Kill, Total number of threads in priority level
INT32 PriorityTotal[PRIORITYLEVELS] 		= {0};		
// Array to skip threads if threas sleep/block, if 0 move to next thread, Avaliable priorities to run
INT32 PriorityAvailable[PRIORITYLEVELS] 	= {0};
// LL for each priority level 
tcbType* PriorityPtr[PRIORITYLEVELS] 		= {0};	
// Pointer to end of LL
tcbType* PriorityLastPtr[PRIORITYLEVELS] 	= {0};



//*********************************************** Basic OS Initilization Functions ********************
//*********************************************** Add CPU/Peripheral Functions to these if desired ********************

/** SetInitialStack
 *	@brief sets stack to default values
 * @param i tcb thread number
 * 
*/
void SetInitialStack(INT32U i){
  tcbs[i].sp = &Stacks[i][STACKSIZE-16]; // thread stack pointer
  Stacks[i][STACKSIZE-1] = 0x01000000;   // thumb bit
  Stacks[i][STACKSIZE-3] = 0x14141414;   // R14
  Stacks[i][STACKSIZE-4] = 0x12121212;   // R12
  Stacks[i][STACKSIZE-5] = 0x03030303;   // R3
  Stacks[i][STACKSIZE-6] = 0x02020202;   // R2
  Stacks[i][STACKSIZE-7] = 0x01010101;   // R1
  Stacks[i][STACKSIZE-8] = 0x00000000;   // R0
  Stacks[i][STACKSIZE-9] = 0x11111111;   // R11
  Stacks[i][STACKSIZE-10] = 0x10101010;  // R10
  Stacks[i][STACKSIZE-11] = 0x09090909;  // R9
  Stacks[i][STACKSIZE-12] = 0x08080808;  // R8
  Stacks[i][STACKSIZE-13] = 0x07070707;  // R7
  Stacks[i][STACKSIZE-14] = 0x06060606;  // R6
  Stacks[i][STACKSIZE-15] = 0x05050505;  // R5
  Stacks[i][STACKSIZE-16] = 0x04040404;  // R4
}


/** SetThreads
* @brief Set Threads to default values for unused.
* 
*/
void SetThreads(void){
	for (INT8 i = 0; i < NUMTHREADS; i++){
		tcbs[i].status = -1;
		tcbs[i].id = -1;
		tcbs[i].sleepState = 0;
	}
	for (INT8 i = 0; i < PRIORITYLEVELS; i++){
		PriorityPtr[i] = 0;
	}
}
	

/** OS_SleepHandler
 * @brief Update Sleep timer and run through linked list and remove any sleeping threads
 *  @return none
*/
void OS_SleepHandler(void){
	// increment timer for sleep
	OS_SystemTimeMS++;
	
	// run through list of threads and decrement sleep counter
	for(INT32 i=0; i < NUMTHREADS; i++){
		// check if thread not dead and sleeping
		if(((tcbs[i].status != -1) && (tcbs[i].sleepState))){
			tcbs[i].sleepState--;
			if (tcbs[i].sleepState == 0){
				PriorityAvailable[tcbs[i].priority]++;
			}
		}
	}
}

/* OS_SystemTime
 * @brief increment timer, more accurate than 1ms timer
**/
void OS_SystemTimeHandler(void){
	OS_SystemTime++;
}

/** Peripheral_Init
* @brief This function initializes extra IO used by OS, add new inits here
*/
void Peripheral_Init(void){
	// 1 ms timer for OS/ sleep decrement
	Timer0A_Init(&OS_SleepHandler, TIME_1MS, 1);
	// accurate system timer, optional
	Timer0B_Init(&OS_SystemTimeHandler, TIME_1MS, 1);

}


/** OS_SystemtPriority
 *	@brief Set priority of Systick and PendSV (Context Switch handlers)
*/
void OS_SystemPriority(void){
	NVIC_SYS_PRI3_R =(NVIC_SYS_PRI3_R&0x00FFFFFF)|0xD0000000; // priority 6
	NVIC_SYS_PRI3_R =(NVIC_SYS_PRI3_R&0xFF00FFFF)|0x00E00000; // priority 7

}
/** OS_Init
 *	@brief initialize operating system, disable interrupts until OS_Launch
 *	initialize OS controlled I/O: serial, ADC, systick, LaunchPad I/O and timers 
*/
void OS_Init(void){
	OS_DisableInterrupts();
	Peripheral_Init();
	OS_SystemTimeMS = 0;
	OS_SystemTime = 0;
	SetThreads();
	OS_SystemPriority();
	RunPt = &tcbs[0]; 
}




/** @brief  LinkTCB
 *	Add TCB to doubly Linked List, copy pasta ee 312
*/
void LinkTCB(tcbType* newThread){
	// add to Doubly LL, copy pasta
	tcbType* endPtr = RunPt->prev;
	// increment count
	PriorityAvailable[newThread->priority]++;
	newThread->next = RunPt;
	RunPt->prev = newThread;
	newThread->prev = endPtr;
	endPtr->next = newThread;
	
	//increment thread count
	NumOfThreads++;
}


/** @brief  UnLinkTCB
 *	Remove thread form Doubly LL of TCB, copy pasta 
*/
void UnLinkTCB(void){
	//do removal for double LL
	RunPt->prev->next = RunPt->next;
	RunPt->next->prev = RunPt->prev;
	NumOfThreads--;
}



/** AddBlockedToSema4
 *	@brief Add TCB to blocked Linked list of semaphore
 *  @param semaPt ptr to semaphore
*/
void AddBlockedToSemaphore(Sema4Type* semaPt){
	// go to end of LL to add tcb or add to head, copy pasta
	if(semaPt->blockThreads == 0){
		semaPt->blockThreads = RunPt;
		semaPt->blockThreads->nextBlocked = 0;
	}else{
		// go to end
		tcbType* endPtr = semaPt->blockThreads;
		while(endPtr->nextBlocked != 0){
			endPtr = endPtr->nextBlocked;
		}
		endPtr->nextBlocked = RunPt;
		endPtr->nextBlocked->nextBlocked = 0;
	}
}




/** RemoveBlockedFromSemaphore
 *	@brief Remove TCB from blocked list, assuiming thread already blocked, else rip program
 *  @param semaPt ptr to semaphore
 *  @return tcb that is no longer blocked from list
*/
tcbType* RemoveBlockedFromSemaphore(Sema4Type* semaPt){
	// copy pasta ee 312, 
	tcbType* headLink = semaPt->blockThreads;
	semaPt->blockThreads = headLink->nextBlocked;
	return headLink;
}


/** UnBlockTCB
 *	@brief Remove TCB from blocked list
 *  @param semaPt ptr to semaphore
*/
void UnBlockTCB(Sema4Type* semaPt){
	tcbType* blocked = RemoveBlockedFromSemaphore(semaPt);
	LinkTCB(blocked);
	blocked->sema4Blocked = 0;
}



/** BlockTCB
 *	@brief Add current TCB to blocked list then yield, ratatatat
 *  @param semaPt ptr to semaphore
*/
void BlockTCB(Sema4Type* semaPt){
	RunPt->sema4Blocked = semaPt;
	PriorityAvailable[RunPt->priority]--;
	UnLinkTCB();
	AddBlockedToSemaphore(semaPt);
	OS_Suspend();
}


/** OS_Launch
* @brief This function starts the scheduler and enables interrupts
* @param theTimeSlice period of round robin scheduler

* 
*/
void OS_Launch(INT32U theTimeSlice){
	INT32 pri = 0;
	for (pri = 0; pri < PRIORITYLEVELS; pri++){
		if (PriorityAvailable[pri] > 0)
			break;
	}
	
	RunPt = PriorityPtr[pri];
	SysTick_Init(theTimeSlice);
	StartOS();
}

/** OS_Scheduler
* @brief This function runs next highest priority thread, PRIORITY SCHEDULER
*/
void OS_Scheduler(void){
	INT32 pri = 0;
	NextRunPt = RunPt->next;
	// go through priority list, exit when next highest thread found
	for (pri = 0; pri < PRIORITYLEVELS; pri++){
		if (PriorityAvailable[pri] > 0)
			break;
	}
	
	// ERROR
	
	// check if next priority thread is the same priority
	if (pri == RunPt->priority){	
		NextRunPt = RunPt->nextPriority;
		// Ensure next thread not sleeping 
		while(NextRunPt->sleepState || NextRunPt->sema4Blocked){
			NextRunPt = NextRunPt->nextPriority;
		} 
	// run lower pri task
	}else{
		if (pri != PRIORITYLEVELS){
			NextRunPt = PriorityPtr[pri];
			// Ensure next thread not sleeping 
			while(NextRunPt->sleepState || NextRunPt->sema4Blocked){	
				NextRunPt = NextRunPt->nextPriority;
			}
		}
	}
	
}


/** SysTick_Handler
 * @brief This function decides next thread to run, 
**/
void SysTick_Handler(void){
	OS_Scheduler();
	//go to pendSV
	NVIC_INT_CTRL_R = NVIC_INT_CTRL_PEND_SV; // go to context switch
	//PE1 ^= 0x02;
}


/** OS_Suspend
* @brief This function suspends current thread by forcing context switch call
* 
*/
void OS_Suspend(void)
{
	//OS_Scheduler();
	//NVIC_INT_CTRL_R = NVIC_INT_CTRL_PEND_SV; // go to context switch
	NVIC_ST_CURRENT_R = 0;      // clear timer
	NVIC_INT_CTRL_R = 0x04000000; // go to SysTick Handler
}




/** OS_AddThread
* @brief This function decides next thread to run, now uses priority scheduler
* @param newThread

* 
*/
void OS_AddPriorityThread(tcbType* newThread){
	INT32 pri = newThread->priority;
	
	// Add head priority task
	if(PriorityTotal[pri] == 0){
		PriorityPtr[pri] 		= newThread;
		PriorityLastPtr[pri] 	= newThread;
		newThread->nextPriority = newThread;
	// Add thread to existing LL, put at head so O(1)
	} else{
		newThread->nextPriority 			= PriorityLastPtr[pri]->nextPriority;
		PriorityLastPtr[pri]->nextPriority 	= newThread;
		PriorityPtr[pri] 					= newThread;
	}
	
	//increment counters
	PriorityAvailable[pri]++;
	PriorityTotal[pri]++;
	
}



/** OS_AddThread
* @brief This function decides next thread to run, now uses priority scheduler
* @param task
* @param stackSize
* @param priority
* @return 1-success, 0-fail
* 
*/
INT8 OS_AddThread(void(*task)(void), INT32U priority){
	// check if max thread limit reached
	if(NumOfThreads >= NUMTHREADS){
		return 0; 
	}
	// Start of adding thread
	INT32U sr = StartCritical();
	
	// search for availalbe tcb
	INT8 idxFreeTCB = 0;
	for(idxFreeTCB = 0; idxFreeTCB < NUMTHREADS; idxFreeTCB++){
		//break once found
		if(tcbs[idxFreeTCB].status == -1){
			break;
		}
	}
	
	// check if this is the only thread
	if(NumOfThreads == 0){
		tcbs[NumOfThreads].next = &tcbs[0];
		tcbs[NumOfThreads].prev = &tcbs[0];
		RunPt = &tcbs[0];
		tcbs[0].priority = priority;
	}
	// add thread, treat like linked list, (code from EE312)
	else{ 
		if(RunPt->status == -1 || idxFreeTCB >= NUMTHREADS){
			EndCritical(sr);
			return 0;
		}
		
		//insertion for doubly linked list
		tcbType* endPtr = RunPt->prev; 
		tcbs[idxFreeTCB].next = RunPt;		
		RunPt->prev = &tcbs[idxFreeTCB];
		tcbs[idxFreeTCB].prev = endPtr;
		endPtr->next = &tcbs[idxFreeTCB];
	}
	//init stack and add task to TCB
	SetInitialStack(idxFreeTCB);
	Stacks[idxFreeTCB][STACKSIZE-2] = (INT32)(task); // PC
	
	// init vars of tcb
	tcbs[idxFreeTCB].sleepState = 0;
	tcbs[idxFreeTCB].status = 0; 
	tcbs[idxFreeTCB].id = idxFreeTCB;
	tcbs[idxFreeTCB].priority = priority;
	
	//increment thread count
	NumOfThreads++;
	
	// Go to Priority Scheduler addition
	OS_AddPriorityThread(&tcbs[idxFreeTCB]);
	EndCritical(sr);
	// yay it worked, unless this is buggy, then :(
	return 1;
}



/** OS_IdThread
 *  @brief Get current thread ID
 *  @return id of thread
*/
INT32U OS_IdThread(void){
	return RunPt->id;
}


/** OS_InitSemaphore
 *  @brief Initialize semaphore to given value
 *  @param  semaPt semaphore ptr
*/
void OS_InitSemaphore(Sema4Type *semaPt, INT32 value){	//Occurs once at the start
	INT32U sr = StartCritical();
	semaPt->Value = value;
	semaPt->blockThreads = 0;
	EndCritical(sr);
}



/** OS_Wait
 *  @brief semaphore value decrement
 *  @param  semaPt pointer to semaphore
 *  @return none
*/
void OS_Wait(Sema4Type *semaPt){ // Called at run time to provide synchronization between threads
	
	INT32U sr = StartCritical();
	semaPt->Value--;
	if (semaPt->Value < 0){
		BlockTCB(semaPt);
		}
	EndCritical(sr);
	
}



/** OS_Signal
 * @brief This function(Spinlock) will signal that a mutual exclusion is taking place in a function
 * @param semaPt 
*/
void OS_Signal(Sema4Type *semaPt){
	INT32U sr = StartCritical();
	semaPt->Value++;
	
	if(semaPt->Value < 1){
		UnBlockTCB(semaPt);
	}
	EndCritical(sr);
}



/** OS_bWait
* @brief This function implements binary wait
* @param semaPt semaphore passed in

* 
*/
void OS_bWait(Sema4Type *semaPt){
	DisableInterrupts();
	semaPt->Value--;
	if (semaPt->Value < 0){
		BlockTCB(semaPt);
	}
	EnableInterrupts();

}


/** OS_bSignal
* @brief This function implements binary signal
* @param semaPt semaphore passed in

* 
*/
void OS_bSignal(Sema4Type *semaPt){
	INT32U sr = StartCritical();
	semaPt->Value++;
	if(semaPt->Value < 1){
		UnBlockTCB(semaPt);
	}
	EndCritical(sr);
}


/** OS_Sleep
* @brief This function puts a thread to sleep
* @param sleepTime time to put thread to sleep

* 
*/
void OS_Sleep(INT32U sleepTime){
	OS_DisableInterrupts();
	// add current thread to sleep list like in class
	RunPt->sleepState = sleepTime;
	// Priroity Scheduling
	PriorityAvailable[RunPt->priority]--;
	
	OS_Suspend();
	OS_EnableInterrupts();
}


/** OS_Kill
* @brief This function kill/deletes current thread from schedule
*/
void OS_Kill(void){
	DisableInterrupts();
	// Fix Priority Scheduler and remove TCB from all LL
	tcbType *tmpPtr, *curPtr;
	
	// Killed Thread is head (top) of linked list, EZ, O(1), copy past LL
	if ((RunPt == PriorityLastPtr[RunPt->priority]) && (RunPt == PriorityPtr[RunPt->priority])){ 
		PriorityPtr[RunPt->priority] 		= 0;
		PriorityLastPtr[RunPt->priority] 	= 0;
	// Remove first link in list (O(1), copy pasta LL
	} else if (RunPt == PriorityPtr[RunPt->priority]){	
			PriorityLastPtr[RunPt->priority]->nextPriority = RunPt->nextPriority;
			PriorityPtr[RunPt->priority] = PriorityLastPtr[RunPt->priority]->nextPriority;
	// find thread to delete O(N)
	} else {	
		tmpPtr = PriorityPtr[RunPt->priority];
		curPtr = PriorityPtr[RunPt->priority];
		// dp searjc
		while (curPtr != RunPt){
			tmpPtr = curPtr; 
			curPtr = curPtr->nextPriority; 
		}
		// check to see if last link in list
		if (RunPt == PriorityLastPtr[RunPt->priority]){ 
			tmpPtr->nextPriority = PriorityPtr[RunPt->priority];
			PriorityLastPtr[RunPt->priority] = tmpPtr;
		} else {	
			tmpPtr->nextPriority = curPtr->nextPriority;
		}
	} 
	// OS_Kill(Sikender); why doesnt this work?????
	tcbs[RunPt->id].status = -1;
	NumOfThreads--;
	PriorityTotal[RunPt->priority]--;
	PriorityAvailable[RunPt->priority]--;
	
	OS_Suspend(); 
	EnableInterrupts();
	//should never run
	INT8U errorVar = 0;
	while(1) errorVar++;
}


/** OS_AddPeriodicThread
 * @brief Adds periodic background thread. Cannot spin, sleep, die, rest, etc. cause it's ISR, depends on hardware for number of tasks possible
			No ID for this thread, must have mid-high priority to run properly
 * @param task task to run in background
 * @param  period 
 * @param  priority 5-0 only, else you'll break OS :(
 * @return successful - 1, Fail - 0
*/
INT8 OS_AddPeriodicThread(void(*task)(void), INT32U period, INT32U priority){ 
	static INT8U NumberOfPeriodicTasks = 0;
	
	if (NumberOfPeriodicTasks == 0){
		Timer1A_Init(task, period, priority);
	} else if (NumberOfPeriodicTasks == 1){
		Timer1B_Init(task, period, priority);
	} else if (NumberOfPeriodicTasks == 2){
		Timer2A_Init(task, period, priority);
	} else if (NumberOfPeriodicTasks == 3){
		Timer2B_Init(task, period, priority);
	} else if (NumberOfPeriodicTasks == 4){
		Timer3A_Init(task, period, priority);
	} else if (NumberOfPeriodicTasks == 5){
		Timer3B_Init(task, period, priority);
	} else if (NumberOfPeriodicTasks == 6){
		Timer4A_Init(task, period, priority);
	} else if (NumberOfPeriodicTasks == 7){
		Timer4B_Init(task, period, priority);		
	// no more timers dawg :(
	}else{
		return 0;
	}
	NumberOfPeriodicTasks++;
	return 1;
}



/** OS_AddSW1Task
* @brief This function adds a thread to run and its priority when a button is pressed
* @param task function/thread to run when button pressed
* @param priority
* @return success or fail 
*/

INT8 OS_AddSW1Task(void(*task)(void), INT32U priority){
	SW1_Init(task, priority);
	return 1;	
}


/** OS_AddSW2Task
* @brief This function adds a thread to run and its priority when a button is pressed
* @param task function/thread to run when button pressed
* @param priority
* @return success or fail 
*/

INT8 OS_AddSW2Task(void(*task)(void), INT32U priority){
	SW2_Init(task, priority);
	return 1;	
}


/** OS_Fifo_Init
* Initializes Fifo to be empty, ignored for lab 2, divisible by 2
*/
void OS_Fifo_Init(void){
	INT32U sr = StartCritical();	// is this necessary? cause idk
	Put_Idx = Get_Idx = 0;
	OS_InitSemaphore(&SemaFIFO, 0);
	EndCritical(sr);
}


/** OS_Fifo_Put
* Adds data to FiFo
* @param data
* @return success - 1, Fail - 0

* 
*/
INT8 OS_Fifo_Put(FIFO_t data){
	// check if FiFo Full
	if(SemaFIFO.Value == FIFO_SIZE) return 0;
	
	// add data to FiFo
	OS_FIFO[Put_Idx] = data;
	Put_Idx = (Put_Idx + 1) % FIFO_SIZE;
	
	// signal to show there's data
	OS_Signal(&SemaFIFO);
	return 1;
} 



/** OS_Fifo_Get
* Retrieves data from OS Fifo
* @return 1 for success, 0 for fail
*/
FIFO_t OS_Fifo_Get(void){
	OS_Wait(&SemaFIFO);
	FIFO_t data;
	// check if empty
	if(Put_Idx == Get_Idx) return 0;
	// get and return data
	data = OS_FIFO[Get_Idx];
	Get_Idx = (Get_Idx + 1) % FIFO_SIZE;
	return data;
}


/** OS_Fifo_Size
* @brief Gets current size of FiFo
* @return size of current FIFO buffer
*/
INT32U OS_Fifo_Size(void){
	//return FiFo_Size_Unused; 
	//return FIFO_SIZE;
	INT32U size = SemaFIFO.Value;
	return size;
}

MailBoxType MailBox;

/** OS_MailBox_Init
* @brief Initializes communication channel for OS
*/
void OS_MailBox_Init(void){
	//MailBox.data = -1;
	// set semaphores
	OS_InitSemaphore(&MailBox.Empty, 1); //FML i set this wrong 4AM
	OS_InitSemaphore(&MailBox.Full,  0);
}


/** OS_MailBox_Send
* Enter mail into the Mailbox
* @brief This function will be called from a foreground thread
* It will spin/block if the MailBox contains data not yet received 
* @param data data to put into mailbox
* 
*/
void OS_MailBox_Send(INT32U data){
	// wait then signal to send data, wait until FiFo has space available
	OS_bWait(&MailBox.Empty);
	MailBox.data = data;
	OS_bSignal(&MailBox.Full);
}



/** OS_MailBox_Recv
* Remove mail from the mailbox
* @brief This function will be called from a foreground thread
* It will spin/block if the MailBox is empty
* @return data from Mailbox
* 
*/
INT32U OS_MailBox_Recv(void){
	INT32U data;
	OS_bWait(&MailBox.Full);
	data = MailBox.data;
	OS_bSignal(&MailBox.Empty);
	return data;
}


 
/** OS_Time
 *  @return OS time in 12.5ns
*/
INT32U OS_Time(void){
	return TIMER0_TAR_R;
}


/**OS_TimeDifference
 * @param start
 * @param stop
 * @return time difference 
*/
INT32U OS_TimeDifference(INT32U start, INT32U stop){
	if(start> stop)
		return (start - stop);
	else{
		return (0xFFFFFFFF - stop + start);
	}
}


/** OS_ClearMsTime
 *  Clear ms time
*/
void OS_ClearMsTime(void){
	OS_SystemTimeMS  = 0;
}



/**OS_MsTime
 *  @return time in ms
*/
INT32U OS_ReadMsTime(void){
	return OS_SystemTimeMS;
}

