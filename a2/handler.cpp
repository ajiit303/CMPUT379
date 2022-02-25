#include "handler.h"

// https://pubs.opengroup.org/onlinepubs/009696899/basedefs/stdarg.h.html
#include <stdarg.h>
#include <cstdio>

void WARNING(const char* fmt, ...) {
    va_list  ap;
    fflush (stdout);
    va_start (ap, fmt);  vfprintf (stderr, fmt, ap);  va_end(ap);
}

void FATAL(const char* fmt, ...) {
    va_list  ap;
    fflush (stdout);
    va_start (ap, fmt);  vfprintf (stderr, fmt, ap);  va_end(ap);
    fflush (NULL);
    exit(1);
}