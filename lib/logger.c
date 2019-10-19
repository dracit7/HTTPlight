
#include "logger.h"

static pthread_mutex_t lock;

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

#ifdef SERIAL
  pthread_mutex_lock(&lock);
  logger("[INFO] ");
  logger(buf);
  pthread_mutex_unlock(&lock);
#else
  logger("[INFO] ");
  logger(buf);
#endif

}

// Logging function for error messages
void Error(const char* fmt, ...) {

  va_list args;
  char buf[LOG_BUF_LEN];

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

#ifdef SERIAL
  pthread_mutex_lock(&lock);
  logger("[ERROR] ");
  logger(buf);
  pthread_mutex_unlock(&lock);
#else
  logger("[ERROR] ");
  logger(buf);
#endif

}

// Logging function for debug messages
void Debug(const char* fmt, ...) {

  va_list args;
  char buf[LOG_BUF_LEN];

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

#ifdef SERIAL
  pthread_mutex_lock(&lock);
  logger("[DEBUG] ");
  logger(buf);
  pthread_mutex_unlock(&lock);
#else
  logger("[DEBUG] ");
  logger(buf);
#endif

}