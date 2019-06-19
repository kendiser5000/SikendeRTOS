
//**************************************************************************************
// The following protoypes are functions that were modified / added by Sikender Ashraf
// and Sijin Woo
//**************************************************************************************


#include "retarget.h"


/**
* This function retargets printf
* @author Sikender
* @param ch character
* @param f  pointer to file
* @date 2/04/2019
*/
int fputc(int ch, FILE *f) {
	UART_OutChar(ch);
	return (1);
}


/**
* This function will retarget scanf 
* @author Sikender
* @param f  pointer to file
* @date 2/04/2019
*/
int fgetc (FILE *f) {
	return (UART_InChar());
}



/**
* This function will retarget ferror to UART
* @author Sikender
* @param f  pointer to file
* @date 2/04/2019
*/
int ferror(FILE *f) {
	/* Your implementation of ferror */
	return EOF;
}

