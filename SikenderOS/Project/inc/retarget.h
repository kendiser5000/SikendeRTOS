/**
* @file retarget.h
* @brief Redirects printf to UART 
*/


#ifndef __retarget_H__
#define __retarget_H__

#include <stdio.h>
#include "UART0.h"

int fputc(int ch, FILE *f);


int fgetc (FILE *f);


int ferror(FILE *f);
 

 #endif // __retarget_H__
 

