#include "common.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

// error logging utilities taken from vkcube repo
void noreturn
failv(const char *format, va_list args)
{
   vfprintf(stderr, format, args);
   fprintf(stderr, "\n");
   exit(1);
}

void printflike(1,2) noreturn
fail(const char *format, ...)
{
   va_list args;

   va_start(args, format);
   failv(format, args);
   va_end(args);
}

void printflike(2, 3)
fail_if(int cond, const char *format, ...)
{
   va_list args;

   if (!cond)
      return;

   va_start(args, format);
   failv(format, args);
   va_end(args);
}
