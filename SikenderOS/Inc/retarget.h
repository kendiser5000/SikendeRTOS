/**
* @file retarget.h
* @brief Redirects printf to UART 
*
* @author Gerstlauer
*
* @date 2/04/2019
*/
//**************************************************************************************
// The following protoypes are functions that were modified / added by Sikender Ashraf
// and Sijin Woo
//**************************************************************************************

#ifndef __retarget_H__
#define __retarget_H__

#include <stdio.h>
#include "UART.h"

int fputc(int ch, FILE *f);


int fgetc (FILE *f);


int ferror(FILE *f);
 

 #endif // __retarget_H__
 

