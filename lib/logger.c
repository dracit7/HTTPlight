
#include "logger.h"

void default_logger(const char* msg) {
  printf(msg);
}

// The logging function defined by user
static void (*logger)(const char* msg) = default_logger;

// Provide users an interface to define the logger
void set_logger(void (*log)(const char*)) {
  logger = log;
}

// Logging function
void Log(const char* fmt, ...) {

  va_list args;
  char buf[LOG_BUF_LEN];

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  logger("[INFO] ");
  logger(buf);

}

// Logging function for error messages
void Error(const char* fmt, ...) {

  va_list args;
  char buf[LOG_BUF_LEN];

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  logger("[ERROR] ");
  logger(buf);

}

// Logging function for debug messages
void Debug(const char* fmt, ...) {

  va_list args;
  char buf[LOG_BUF_LEN];

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  logger("[DEBUG] ");
  logger(buf);

}