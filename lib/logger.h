
#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>

#define LOG_BUF_LEN 512

void set_logger(void (*log)(const char*));
void Log(const char*, ...);
void Error(const char*, ...);
void Debug(const char*, ...);

#endif