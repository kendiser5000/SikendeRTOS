// IR.c
// Created On:  04/10/19
// Modified On: 04/10/19
// Created By:  Richard Li
#include "ADC.h"
#include "IR.h"
#include <stdio.h>
#include <stdlib.h>

#define FILTERSIZE	3
#define skewRate	0
#define fs 			100
uint16_t medianFilter[FILTERSIZE];
int middle;
int buffIndex;

void IR_Task(unsigned long data);
long ADC2millimeter(long adcSample);

void IR_Init(uint32_t channelNum){
	//ADC0_InitTimer2ATriggerSeq3(channelNum, 80000000 / fs);
	ADC_Collect(channelNum, fs, IR_Task);
	middle = FILTERSIZE / 2;
}

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

//get IR filtered value
int IR_getValue(void){
	//qsort(medianFilter, FILTERSIZE, sizeof(int), cmpfunc);
	return ADC2millimeter(medianFilter[middle]);
}

void IR_Task(unsigned long data){
	medianFilter[buffIndex] = data;
  buffIndex = (buffIndex + 1) % FILTERSIZE;
}

//------------ADC2millimeter------------
// convert 12-bit ADC to distance in 1mm
// it is known the expected range is 100 to 800 mm
// Input:  adcSample 0 to 4095
// Output: distance in 1mm
long ADC2millimeter(long adcSample){
 if(adcSample<494) return 799; // maximum distance 80cm
 return (268130/(adcSample-159));
}

