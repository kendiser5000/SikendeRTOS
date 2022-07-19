# SikendeRTOS
Embedded Real-Time Operating System for ARMv7m Microcontrollers(supports TI TM4C123)

<a href="https://kendiser5000.github.io/SikendeRTOS/index.html" title="Doxygen Docs">Doxygen Docs</a>

<a href="https://github.com/kendiser5000/SikendeRTOS" title="Source Code">Source Code</a>

# Overview
The SikendeRTOS project is my try at making an RTOS from scratch that is HW independent. Also, documenting the code throughout the process (Show me the ways Doxygen!). The goal of this is to be a learning and debugging exercise. Also, I hope to use this as a base for my future projects :grinning:

Atm I use the Keil IDE; however, this can be ported over to a makefile based project (Do this future me!)

The API functions for the OS are documented by Doxygen and are all included in [OS.h](https://www.sikenderaliashraf.com/SikendeRTOS/html/_o_s_8c.html)

The parameters for the OS can be configured in [OSConfig.h](https://www.sikenderaliashraf.com/SikendeRTOS/html/_o_s_config_8h.html). **Make sure to adjust the timer values according to the HW used!**

# Project Structure
The project/file structure is broken down into the following:
- docs: Compiled Doxygen output
- html: Doxygen Output
- SikenderOS: contains OS files
  - HAL: HW dependencies
  - CMSIS: startup files for SoC (usually supplied by the manufacturer)
  - Project: Source code for SW using RTOS
  - RTOS: Source code for RTOS

# Basic Design
An RTOS typically has four design principles. These are the ones I implemented for this RTOS:
- Task management and scheduling
- Interrupt servicing;
- Inter-process communication and synchronization
- ~~Memory Management~~

This RTOS does not contain support for memory management due to the constraints of the HW I am testing with and the amount of testing time it requires. I will consider adding it in the future if any of the projects need it. Many issues could arise from using/implementing dynamic memory one(e.g. corrupt memory, loss of real-time system).

## RTOS Design:

There are a lot of visuals in the doxygen docs that I recommend looking at to get a better understanding of the RTOS setup.

This RTOS uses blocking semaphores. I initially used spinning semaphores but did not like the amount of time the OS idled.

Sleep is also implemented for tasks using a HW timer (ideally low power timer).

## Scheduler
The algorithm used for determining the next task is based on its priority level (preemptive scheduler). A higher priority task will always take precedent. The threads are stored in a LL and the OS cycles through N threads to determine which thread should run. 
If the threads are at an equal priority level (Round Robin), it will give each thread an equal amount of time to run. As a result, a higher number of threads potentially results in higher latency between tasks due to increased time iterating through threads. Given the use cases for the RTOS, this is not a concern.

### Scheduling Algorithms Evaluated:
- Round Robin: all threads get equal time
- Cooperative: threads can release themselves after it is done, which requires trusting the user
- Priority (Preemptive): each thread is assigned a priority
  - Beware priority inversion
- Multilevel Feedback Queue
  - Rules:
    - All threads start at level 0 priority (highest)
    - Highest Runs: Prio(Ti) < Prio(Tj) timeslice
    - Equal take turns (Round Robin)
    - True Accounting: Thread priority decreases when timeslice is used up (accumulated time)
    - Periodic Reset of priority
    - The thread that runs more often reduces priority for each context switch
  - Metrics:
    - Turnaround Time (Tfinish Tarrival):  optimized by Shortest Remaining Time First (SRTF), good for batch
    - Response Time (Interactive Systems): Time execution Time arrival, Round Robin Optimized
  
  I decided to use a hybrid of priority and round robin because it was simple to implement, thus easier to debug. 



# Acronyms Used
- RTOS: Real-Time Operating System
- HW: Hardware
- SW: Software
- SoC: System on Chip
- HAL: Hardware Abstraction Layer
