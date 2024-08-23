#ifndef COMMON_H_
#define COMMON_H_

#include <stdarg.h>
#include <stdnoreturn.h>

#define printflike(a, b) __attribute__((format(printf, (a), (b))))

void noreturn failv(const char *format, va_list args);
void noreturn fail(const char *msg, ...) printflike(1, 2);
void fail_if(int cond, const char *msg, ...) printflike(2, 3);

#endif // COMMON_H_
