#ifndef MY_ASSERT_H
#define MY_ASSERT_H

#include <stdio.h>
#include <stdlib.h>

const int DEBUG_SOFT = 0;

#ifdef MY_ASSERT
	#undef MY_ASSERT
#endif

#ifndef NDEBUG
	#define MY_ASSERT(instruction, message) \
	if (instruction)\
	{\
		printf ("\v " #message "\n\n");\
		printf ("An error occurred in the file: %s \n\n"\
				"In line:                       %d \n\n"\
				"In function:                   %s \n\n",\
				__FILE__, __LINE__, __PRETTY_FUNCTION__);\
		if (DEBUG_SOFT == 0) abort();\
	}			
#endif

#ifdef NDEBUG 
	#define MY_ASSERT(instruction, message)
#endif

#endif