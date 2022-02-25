// https://docs.fileformat.com/programming/h/ (Header Guards)
#include <stdarg.h>

#ifndef HANDLER_H
#define HANDLER_H

/**
* The two methods are a taken from Lab 1 in starter.c
*/

void WARNING(const char* fmt, ...);
void FATAL(const char* fmt, ...);

#endif