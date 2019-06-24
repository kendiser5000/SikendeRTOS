/**
* @file OS.c
* @brief Contains functions to run OS
* @date 2/04/2019
*/

 
#include "OS.h"
#include "cpu.h"
#include "startup.h"
#include "cpu_vars.h"


// OS ASM functions
void StartOS(void);
void OS_EnableInterrupts(void);
void OS_DisableInterrupts(void);


/*! @var uint32_t Stacks
    @brief Contains all the stacks for each thread
*/
uint32_t Stacks[NUMTHREADS][STACKSIZE];


/*! @var uint8_t numOfThreads
    @brief number of threads running
*/
static uint8_t NumOfThreads = 0;


/*! @var uint8_t numOfThreads
    @brief number of threads running
*/
uint32_t OS_SystemTime = 0;

//function prototypes in os.c
void Timer3A_Init(unsigned long period);
void Timer0A_Init(void);

unsigned long OS_SystemTimeMS;


/** @var Thread Control Block
 * @brief Structure for each thread, helps with context switching, Now Doubly linked List
*/
struct Tcb{
	/*@{*/
	uint32_t *sp;			/**< pointer to stack (valid for threads not running */
	// double LL
	struct Tcb* next;		/**< Pointer to next TCB */
	struct Tcb* prev;
	// basic properties
	int id;					/**< ID number of thread, negative if unused */
	int status;				/**< status of thread: -1: unused, 1: used */
	uint8_t priority;		/**< priority of thread, 0-5 */
	uint32_t sleepState;		/**< sleep state */
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

uint32_t FiFo_Size_Unused;
uint32_t OS_FiFo[FIFO_SIZE];
uint16_t Get_Idx = 0, Put_Idx = 0;
Sema4Type SemaFIFO;


//************* PRIORITY SCHEDULING GLOBALS AND ARRAYS**************************************************************************
//Value > 0, else 0 if empty, change upon OS_Kill, Total number of threads in priority level
int32_t PriorityTotal[PRIORITYLEVELS] 		= {0};		
// Array to skip threads if threas sleep/block, if 0 move to next thread, Avaliable priorities to run
int32_t PriorityAvailable[PRIORITYLEVELS] 	= {0};
// LL for each priority level 
tcbType* PriorityPtr[PRIORITYLEVELS] 		= {0};	
// Pointer to end of LL
tcbType* PriorityLastPtr[PRIORITYLEVELS] 	= {0};



//*********************************************** Basic OS Initilization Functions ********************
//*********************************************** Add CPU/Peripheral Functions to these if desired ********************

/** SetInitialStack
 *	sets stack to default values
 * @parameter i tcb thread number
 * @date 2/04/2019
*/
void SetInitialStack(int i){
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
* Set Threads to default values for unused.
* @date 2/04/2019
*/
void SetThreads(void){
	for (int i = 0; i < NUMTHREADS; i++){
		tcbs[i].status = -1;
		//tcbs[i].sema4Blocked = 0;
		tcbs[i].id = -1;
		//tcbs[i].priority = 0x00;
		tcbs[i].sleepState = 0;
	}
	for (int i = 0; i < PRIORITYLEVELS; i++){
		PriorityPtr[i] = 0;
	}
}
			


/** Peripheral_Init
* This function initializes extra IO used by OS, add new inits here

* @date 2/04/2019
*/
void Peripheral_Init(void){
	Timer3A_Init(TIME_1MS);
	//Timer0A_Init();

}



/** @brief SysTick_Init
 *	initialize Systick interrupt and values 
*/
void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
	NVIC_ST_CURRENT_R = 0;      // any write to current clears it
	NVIC_SYS_PRI3_R =(NVIC_SYS_PRI3_R&0x00FFFFFF)|0xD0000000; // priority 6
	NVIC_SYS_PRI3_R =(NVIC_SYS_PRI3_R&0xFF00FFFF)|0x00E00000; // priority 7
}




/** @brief  OS_Init
 *	initialize operating system, disable interrupts until OS_Launch
 *	initialize OS controlled I/O: serial, ADC, systick, LaunchPad I/O and timers 
*/
void OS_Init(void){
	OS_DisableInterrupts();
	Peripheral_Init();
	OS_SystemTimeMS = 0;
	OS_SystemTime = 0;
	SetThreads();
	SysTick_Init();
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
 *	Add TCB to blocked Linked list of semaphore
 *  @parameter semaPt ptr to semaphore
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
 *	Remove TCB from blocked list, assuiming thread already blocked, else rip program
 *  @parameter semaPt ptr to semaphore
 *  @return tcb that is no longer blocked from list
*/
tcbType* RemoveBlockedFromSemaphore(Sema4Type* semaPt){
	// copy pasta ee 312, 
	tcbType* headLink = semaPt->blockThreads;
	semaPt->blockThreads = headLink->nextBlocked;
	return headLink;
}


/** UnBlockTCB
 *	Remove TCB from blocked list
 *  @parameter semaPt ptr to semaphore
*/
void UnBlockTCB(Sema4Type* semaPt){
	tcbType* blocked = RemoveBlockedFromSemaphore(semaPt);
	LinkTCB(blocked);
	blocked->sema4Blocked = 0;
}



/** BlockTCB
 *	Add current TCB to blocked list then yield, ratatatat
 *  @parameter semaPt ptr to semaphore
*/
void BlockTCB(Sema4Type* semaPt){
	RunPt->sema4Blocked = semaPt;
	PriorityAvailable[RunPt->priority]--;
	UnLinkTCB();
	AddBlockedToSemaphore(semaPt);
	OS_Suspend();
}


/** OS_Launch
* This function starts the scheduler and enables interrupts
* @TimeSlice period of round robin scheduler

* @date 2/04/2019
*/
void OS_Launch(unsigned long theTimeSlice){
	int32_t pri = 0;
	for (pri = 0; pri < PRIORITYLEVELS; pri++){
		if (PriorityAvailable[pri] > 0)
			break;
	}
	
	RunPt = PriorityPtr[pri];
	
	NVIC_ST_RELOAD_R = theTimeSlice - 1; // reload value
	NVIC_ST_CTRL_R = 0x00000007; // enable, core clock and interrupt arm
	StartOS();
}

/** OS_Scheduler
* This function runs next highest priority thread, PRIORITY SCHEDULER

* @date 2/04/2019
*/
void OS_Scheduler(void){
	int32_t pri = 0;
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
* This function decides next thread to run, 

* @date 2/04/2019
*/
void SysTick_Handler(void){
	OS_Scheduler();
	//go to pendSV
	NVIC_INT_CTRL_R = NVIC_INT_CTRL_PEND_SV; // go to context switch
	//PE1 ^= 0x02;
}


/** OS_Suspend
* This function suspends current thread by forcing context switch call

* @date 2/04/2019
*/
void OS_Suspend(void)
{
	//OS_Scheduler();
	//NVIC_INT_CTRL_R = NVIC_INT_CTRL_PEND_SV; // go to context switch
	NVIC_ST_CURRENT_R = 0;      // clear timer
	NVIC_INT_CTRL_R = 0x04000000; // go to SysTick Handler
}




/** OS_AddThread
* This function decides next thread to run, now uses priority scheduler
* @parameter newThread

* @date 2/04/2019
*/
void OS_AddPriorityThread(tcbType* newThread){
	int32_t pri = newThread->priority;
	
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
* This function decides next thread to run, now uses priority scheduler
* @parameter task
* @paramter stackSize
* @parameter priority
* @return 1-success, 0-fail

* @date 2/04/2019
*/
int OS_AddThread(void(*task)(void), unsigned long stackSize, unsigned long priority){
	// check if max thread limit reached
	if(NumOfThreads >= NUMTHREADS){
		return 0; 
	}
	// Start of adding thread
	uint32_t sr = StartCritical();
	
	// search for availalbe tcb
	int idxFreeTCB = 0;
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
	Stacks[idxFreeTCB][STACKSIZE-2] = (int32_t)(task); // PC
	
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
unsigned long OS_IdThread(void){
	return RunPt->id;
}


/** OS_InitSemaphore
 *  Initialize semaphore to given value
 *  @param  semaPt semaphore ptr
*/
void OS_InitSemaphore(Sema4Type *semaPt, long value){	//Occurs once at the start
	uint32_t sr = StartCritical();
	semaPt->Value = value;
	semaPt->blockThreads = 0;
	EndCritical(sr);
}



/** OS_Wait
 *  semaphore value decrement
 *  @param  semaPt pointer to semaphore
 *  @return none
*/
void OS_Wait(Sema4Type *semaPt){ // Called at run time to provide synchronization between threads
	
	uint32_t sr = StartCritical();
	semaPt->Value--;
	if (semaPt->Value < 0){
		BlockTCB(semaPt);
		}
	EndCritical(sr);
	
}



/** OS_Signal
 * This function(Spinlock) will signal that a mutual exclusion is taking place in a function
 
 * @date 2/04/2019
 * @counter counter
*/
void OS_Signal(Sema4Type *semaPt){
	uint32_t sr = StartCritical();
	semaPt->Value++;
	
	if(semaPt->Value < 1){
		UnBlockTCB(semaPt);
	}
	EndCritical(sr);
}



/** OS_bWait
* This function implements binary wait
* @parameter semaPt semaphore passed in

* @date 2/04/2019
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
* This function implements binary signal
* @parameter semaPt semaphore passed in

* @date 2/04/2019
*/
void OS_bSignal(Sema4Type *semaPt){
	uint32_t sr = StartCritical();
	semaPt->Value++;
	if(semaPt->Value < 1){
		UnBlockTCB(semaPt);
	}
	EndCritical(sr);
}


/** OS_Sleep
* This function puts a thread to sleep
* @parameter sleepTime time to put thread to sleep

* @date 2/04/2019
*/
void OS_Sleep(unsigned long sleepTime){
	DisableInterrupts();
	// add current thread to sleep list like in class
	RunPt->sleepState = sleepTime;
	// Priroity Scheduling
	PriorityAvailable[RunPt->priority]--;
	
	OS_Suspend();
	OS_EnableInterrupts();
}



/** Timer3A_MS
 *	Sleep timer for threads
 *  @param  period given in system time units (12.5ns)
 *  @return none
*/
void Timer3A_Init(unsigned long period){
	// copy pasta
	long sr;
	sr = StartCritical();
	SYSCTL_RCGCTIMER_R |= 0x08;   // 0) activate TIMER3
	TIMER3_CTL_R = 0x00000000;    // 1) disable TIMER3A during setup
	TIMER3_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
	TIMER3_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
	TIMER3_TAILR_R = period - 1;   // 4) reload value
	TIMER3_TAPR_R = 0;            // 5) bus clock resolution
	TIMER3_ICR_R = 0x00000001;    // 6) clear TIMER3A timeout flag
	TIMER3_IMR_R = 0x00000001;    // 7) arm timeout interrupt
	NVIC_PRI8_R = (NVIC_PRI8_R&0x00FFFFFF)|0x20000000; // 8) priority 1
	
	// interrupts enabled in the main program after all devices initialized
	// vector number 51, interrupt number 35
	NVIC_EN1_R = 1<<(35-32);      // 9) enable IRQ 35 in NVIC
	TIMER3_CTL_R = 0x00000001;    // 10) enable TIMER3A
	EndCritical(sr);
}


/** Timer3A_Handler
 *	Update Sleep timer and run through linked list and remove any sleeping threads
 *  @return none
*/
void Timer3A_Handler(void){
	TIMER3_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER3A timeout
	// increment timer for sleep
	OS_SystemTimeMS++;
	
	// run through list of threads and decrement sleep counter
	for(int i=0; i<NUMTHREADS; i++){
		// check if thread not dead and sleeping
		if(((tcbs[i].status != -1) && (tcbs[i].sleepState))){
			tcbs[i].sleepState--;
			if (tcbs[i].sleepState == 0){
				PriorityAvailable[tcbs[i].priority]++;
			}
		}
	}
}


/** OS_Kill
* This function kill/deletes current thread from schedule

* @date 2/04/2019
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
			tmpPtr = curPtr; //
			curPtr = curPtr->nextPriority; //
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
	uint8_t errorVar = 0;
	while(1) errorVar++;
}


void (*Periodic_Thread1)(void); 
uint32_t Period1, Period1Counter=0;
/** PeriodicThread1_Init
 *	Setup periodic thread 1
 *  @param  period period of task
 *  @param  priority of thread
*/
void PeriodicThread1_Init(uint32_t period, uint32_t priority){
	// copy pasta
	long sr;
	sr = StartCritical();
	SYSCTL_RCGCTIMER_R |= 0x10;   	// 0) activate TIMER4
	TIMER4_CTL_R = 0x00000000;    	// 1) disable TIMER4A during setup
	TIMER4_CFG_R = 0x00000000;    	// 2) configure for 32-bit mode
	TIMER4_TAMR_R = 0x00000002;   	// 3) configure for periodic mode, default down-count settings
	TIMER4_TAILR_R = period - 1;    	// 4) reload value
	TIMER4_TAPR_R = 0;            	// 5) bus clock resolution
	TIMER4_ICR_R = 0x00000001;    	// 6) clear TIMER4A timeout flag
	TIMER4_IMR_R = 0x00000001;    	// 7) arm timeout interrupt
	priority = (priority & 0x07) << 21; // mask priority (nvic bits 23-21)
	NVIC_PRI17_R = (NVIC_PRI17_R&0xF00FFFFF);
	NVIC_PRI17_R = (NVIC_PRI17_R | priority); // 8) priority
	Period1 = period;
	// interrupts enabled in the main program after all devices initialized
	// vector number 51, interrupt number 35
	NVIC_EN2_R = 1<<(70-64);      // 9) enable IRQ 70 in NVIC 
	TIMER4_CTL_R = 0x00000001;    // 10) enable TIMER4A
	EndCritical(sr);
}

/** Timer4A_Handler
 *	Run periodic task 1
*/
void Timer4A_Handler(void){
	TIMER4_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER4A timeout
	(*Periodic_Thread1)(); 
	
	
}



void (*Periodic_Thread2)(void); 
uint32_t Period2, Period2Counter=0;
/** PeriodicThread1_Init
 *	Setup periodic thread 1
 *  @param  period period of task
 *  @param  priority of thread
*/
void PeriodicThread2_Init(uint32_t period, uint32_t priority){
	// copy pasta
	long sr;
	sr = StartCritical();
	SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
	Period2 = period;
	TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
	TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
	TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
	TIMER1_TAILR_R = period-1;    // 4) reload value
	TIMER1_TAPR_R = 0;            // 5) bus clock resolution
	TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
	TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
	
	priority = (priority & 0x07) << 13; // mask priority (nvic bits 15-13)
	NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)| priority; // 8) priority 
	
	// interrupts enabled in the main program after all devices initialized
	// vector number 37, interrupt number 21
	NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
	TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
	EndCritical(sr);
}

/** Timer4A_Handler
 *	Run periodic task 1
*/
void Timer1A_Handler(void){
	TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER4A timeout
	// (*Periodic_ThreadHook)();
	(*Periodic_Thread1)(); 
	
}



/** OS_AddPeriodicThread
 * Adds periodic background thread. Cannot spin, sleep, die, rest, etc. cause it's ISR
 * No ID for this thread (sorry Sijin D:), must have mid-high priority to run properly
 * @param task task to run in background
 * @param  period 
 * @param  priority 5-0 only, else you'll break OS :(
 * @return successful - 1, Fail - ?
*/
int OS_AddPeriodicThread(void(*task)(void), unsigned long period, unsigned long priority){ 
	static uint8_t NumberOfPeriodicTasks = 0;
	
	if (NumberOfPeriodicTasks == 0){
		Periodic_Thread1 = task;
		PeriodicThread1_Init(period, priority);
		Period1 = period;
	} else if (NumberOfPeriodicTasks == 1){
		//Add Periodic Thread 2
		Periodic_Thread2 = task;          // user function
		PeriodicThread2_Init(period, priority);
		Period2 = period;
	// no more timers dawg :(
	}else{
		return 0;
	}
	
	NumberOfPeriodicTasks++;
	return 1;
}


#define PF_BUTTON1 0x10
uint32_t Sw1Pri;
/** OS_SW1_Init
* This functionsets up button SW 1 to run task, PF4 built in button (can be changed)
* @parameter priority

* @date 2/04/2019
*/
void SW1_Init(uint32_t priority){
	
	// copy pasta
	volatile uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x00000020; 			// (a) activate clock for port F
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTF_CR_R = PF_BUTTON1;           	// allow changes to PF4, Needed?
	GPIO_PORTF_DIR_R &= ~PF_BUTTON1;    		// make PF4 button
	GPIO_PORTF_PUR_R |= PF_BUTTON1;				// Pull up for Button
	GPIO_PORTF_AFSEL_R &= ~PF_BUTTON1;  		// disable alt funct on PF4
	GPIO_PORTF_DEN_R |= PF_BUTTON1;     		// enable digital I/O on PF4   
	GPIO_PORTF_PCTL_R &= ~0x000F0000; 			// configure PF4 as GPIO
	GPIO_PORTF_AMSEL_R = 0;       				// disable analog functionality on PF
	GPIO_PORTF_IS_R &= ~PF_BUTTON1;     		// PF4 is edge-sensitive
	GPIO_PORTF_IBE_R &= ~PF_BUTTON1;     		// Not double edge
	GPIO_PORTF_IEV_R &= ~PF_BUTTON1;     		// PF4 only falling edge
	GPIO_PORTF_ICR_R = PF_BUTTON1;      		// arm interrupt?
	GPIO_PORTF_IM_R |= PF_BUTTON1;      		// arm interrupt on PF4
	priority = (priority & 0x07) << 21;			// set NVIC priority bit (21-23)
	Sw1Pri = priority;
	NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF); 	
	NVIC_PRI7_R = (NVIC_PRI7_R | priority); 	
	
	NVIC_EN0_R = 0x40000000;      				// enable interrupt 30 in NVIC 	
}


/** OS_AddSW1Task
* This function adds a thread to run and its priority when a button is pressed
* @parameter task function/thread to run when button pressed
* @parameter priority
* @return success or fail

* @date 2/04/2019
*/
void (*SW1_Task)(void);
uint32_t SW1_Priority;
uint32_t LastSW1;
int OS_AddSW1Task(void(*task)(void), unsigned long priority){
	SW1_Task = task;
	SW1_Init(priority);
	LastSW1 = PF_BUTTON1;
	//SW1_Priority = priority;
	return 1;
	
}



#define PF_BUTTON2 0x01
/** OS_SW1_Init
* This functionsets up button SW 1 to run task, PF4 built in button (can be changed)
* @parameter priority

* @date 2/04/2019
*/
void SW2_Init(uint32_t priority){
	// copy pasta
	SYSCTL_RCGCGPIO_R |= 0x00000020; 			// activate clock for port F
	while((SYSCTL_PRGPIO_R & 0x00000020) == 0){};
	GPIO_PORTF_LOCK_R = 0x4C4F434B; 			// unlock GPIO Port F, just like Sijin unlocks my heart <3
	GPIO_PORTF_CR_R = 0x1F;
	GPIO_PORTF_DIR_R &= ~PF_BUTTON2;    		// PF0 is button
	GPIO_PORTF_PUR_R |= PF_BUTTON2;     		// enable weak pull-up 
	GPIO_PORTF_AFSEL_R &= ~PF_BUTTON2;  		// disable alt funct 
	GPIO_PORTF_DEN_R |= PF_BUTTON2;     		// enable digital I/O   
	GPIO_PORTF_PCTL_R &= ~0x0000000F; 			// configure PF0 as GPIO
	GPIO_PORTF_AMSEL_R = 0;       				// disable analog functionality
	GPIO_PORTF_IS_R &= ~PF_BUTTON2;     		// Edge-sensitive
	GPIO_PORTF_IBE_R &= ~PF_BUTTON2;     		// not double edge sensitve
	GPIO_PORTF_IEV_R &= ~PF_BUTTON2;     		// falling edge sensitve
	GPIO_PORTF_ICR_R = PF_BUTTON2;      		// clear flag
	GPIO_PORTF_IM_R |= PF_BUTTON2;      		// arm interrupt

	priority = (priority & 0x07) << 21;						// NVIC priority bit (21-23)

	NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF); 				
	NVIC_PRI7_R = (NVIC_PRI7_R | priority); 
	NVIC_EN0_R = 0x40000000;      							// enable interrupt 30 in NVIC 	
	
}


/** OS_AddSW2Task
* This function adds a thread and runs thread when a button is pressed
* @parameter task function/thread to run when button pressed
* @parameter priority
* @return success or fail

* @date 2/04/2019
*/
void (*SW2_Task)(void);
uint32_t SW2_Priority;
uint32_t LastSW2;
int OS_AddSW2Task(void(*task)(void), unsigned long priority){
	SW2_Task = task;
	SW2_Init(priority);
	//SW2_Priority = priority;
	LastSW2 = PF_BUTTON2;
	return 1;
}


/** @brief  Debounce_SW1
 *	debounce switch 1 with sleep, easier than timer maybe?
 
 * @date 2/04/2019
*/
void SW1_Debounce(void){
	OS_Sleep(15); 
	LastSW1 = PF_BUTTON1;
	GPIO_PORTF_ICR_R = PF_BUTTON1;      // clear flag
	GPIO_PORTF_IM_R |= PF_BUTTON1;      
	OS_Kill();					// done debouncing yo
}


/** @brief  Debounce_SW2
 *	debounce switch 2 with sleep, easier than timer maybe?
 
 * @date 2/04/2019
*/
void SW2_Debounce(void){
	OS_Sleep(15); // sleep for 10ms
	LastSW2 = PF_BUTTON2;
	GPIO_PORTF_ICR_R = PF_BUTTON2;      // clear flag
	GPIO_PORTF_IM_R |= PF_BUTTON2;      
	OS_Kill();					// done debouncing yo
}


/** GPIOPortF_Handler
 *	Add thread for a button press
 *  @param  none
 *  @return none
*/
void GPIOPortF_Handler(void){
	// sw1 pressed
	if(GPIO_PORTF_RIS_R & PF_BUTTON1){    
		if (LastSW1 == PF_BUTTON1){
			(*SW1_Task)();
			// LastSW1 = 0;
		}
		//GPIO_PORTF_IM_R &= ~PF_BUTTON1;     
		GPIO_PORTF_ICR_R = PF_BUTTON1;      // clear flag
		// debounce dat hoe
		int32_t status = OS_AddThread(&SW1_Debounce, 128, SW1_Priority);
		// cant make thread, probable too many threads, chill out my guy (@TA)
		if(status == 0){ 
			GPIO_PORTF_ICR_R = PF_BUTTON1;      // clear flag
			GPIO_PORTF_IM_R |= PF_BUTTON1;      
		}
	}
	
	// sw2 pressed, cant do else in case both pressed
	if(GPIO_PORTF_RIS_R & PF_BUTTON2){		// SW2 pressed
		if (LastSW2 == PF_BUTTON2){
			(*SW2_Task)();
			// LastSW2 = 0;
		}
		//GPIO_PORTF_IM_R &= ~PF_BUTTON2;     // disarm interrupt
		GPIO_PORTF_ICR_R = PF_BUTTON2;      // clear flag
		OS_AddThread(&SW2_Debounce, 128, SW2_Priority);
		int32_t status = OS_AddThread(&SW2_Debounce, 128, SW2_Priority);
		// cant make thread, probable too many threads, chill out my guy (@TA)
		if(status == 0){ 
			GPIO_PORTF_ICR_R = PF_BUTTON2;      // clear flag
			GPIO_PORTF_IM_R |= PF_BUTTON2;      
		}
	}
}


/**
 * OS_FIFO
 * FiFo for OS, 
 */

/** OS_Fifo_Init
* Initializes Fifo to be empty, ignored for lab 2, divisible by 2
* @parameter size size of FiFo

* @date 2/04/2019
*/
void OS_Fifo_Init(void){
	uint32_t sr = StartCritical();	// is this necessary? cause idk
	Put_Idx = Get_Idx = 0;
	OS_InitSemaphore(&SemaFIFO, 0);
	EndCritical(sr);
}


/** OS_Fifo_Put
* Adds data to FiFo
* @parameter data
* @return success - 1, Fail - 0

* @date 2/04/2019
*/
int OS_Fifo_Put(unsigned long data){
	// check if FiFo Full
	if(SemaFIFO.Value == FIFO_SIZE) return 0;
	
	// add data to FiFo
	OS_FiFo[Put_Idx] = data;
	Put_Idx = (Put_Idx + 1) % FIFO_SIZE;
	
	// signal to show there's data
	OS_Signal(&SemaFIFO);
	return 1;
} 



/** OS_Fifo_Get
* Retrieves data from OS Fifo
* @return 1 for success, 0 for fail

* @date 2/04/2019
*/
unsigned long OS_Fifo_Get(void){
	OS_Wait(&SemaFIFO);
	uint32_t data;
	// check if empty
	if(Put_Idx == Get_Idx) return 0;
	// get and return data
	data = OS_FiFo[Get_Idx];
	Get_Idx = (Get_Idx + 1) % FIFO_SIZE;
	return data;
}


/** OS_Fifo_Size
* Gets current size of FiFo
* @parameter size size of FiFo

* @date 2/04/2019
*/
long OS_Fifo_Size(void){
	//return FiFo_Size_Unused; 
	//return FIFO_SIZE;
	uint32_t size = SemaFIFO.Value;
	return size;
}

MailBoxType MailBox;

/** OS_MailBox_Init
* Initializes communication channel for OS
* @date 2/04/2019
*/
void OS_MailBox_Init(void){
	//MailBox.data = -1;
	// set semaphores
	OS_InitSemaphore(&MailBox.Empty, 1); //FML i set this wrong 4AM
	OS_InitSemaphore(&MailBox.Full,  0);
}


/** OS_MailBox_Send
* Enter mail into the Mailbox
* This function will be called from a foreground thread
* It will spin/block if the MailBox contains data not yet received 
* @parameter data data to put into mailbox
* @date 2/04/2019
*/
void OS_MailBox_Send(unsigned long data){
	// wait then signal to send data, wait until FiFo has space available
	OS_bWait(&MailBox.Empty);
	MailBox.data = data;
	OS_bSignal(&MailBox.Full);
}



/** OS_MailBox_Recv
* Remove mail from the mailbox
* This function will be called from a foreground thread
* It will spin/block if the MailBox is empty
* @return data from Mailbox
* @date 2/04/2019
*/
unsigned long OS_MailBox_Recv(void){
	uint32_t data;
	OS_bWait(&MailBox.Full);
	data = MailBox.data;
	OS_bSignal(&MailBox.Empty);
	return data;
}


 
/** OS_Time
 *  @return OS time in 12.5ns
*/
unsigned long OS_Time(void){
	return TIMER0_TAR_R;
}


/**OS_TimeDifference
 * @param start
 * @param stop
 * @return time difference 
*/
unsigned long OS_TimeDifference(unsigned long start, unsigned long stop){
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
unsigned long OS_MsTime(void){
	return OS_SystemTimeMS;
}



/** Timer0A_Init
 *  Setup 12.5ns timer
 *  @return time in ms
*/
void Timer0A_Init(void){long sr;
	sr = StartCritical(); 
	SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
	TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
	TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
	TIMER0_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
	TIMER0_TAILR_R = 0xFFFFFFFF - 1;    // 4) reload value
	TIMER0_TAPR_R = 0;            // 5) bus clock resolution
	TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
	TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
	NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x00000000; // 8) priority 1
	// interrupts enabled in the main program after all devices initialized
	// vector number 35, interrupt number 19
	NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
	TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
	EndCritical(sr);
}



/** Timer0A_Handler
 *  Add to OS time 12.5ns
 *  @return time in ms
*/
void Timer0A_Handler(void){
	TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
	OS_SystemTime++;
}

