
#ifndef ERROR_H
#define ERROR_H

#include <errno.h>

/*
 * Error codes
 */

enum error_codes {
  
  E_UNRECOG = 1,

  // Post parsing
  E_INVALID_ARGS,
  E_INVALID_PORT,
  E_INVALID_IP,
  E_INVALID_POST,

  // socket
  E_FILE_NOT_FOUND,
  E_SOCK_CREATE_FAILED,
  E_BIND,
  E_LISTEN,
  E_CONNECT,
  E_NO_MEM,
  E_NO_IDLE_THREAD,

  // thread pool
  E_LOCK_FAILED,
  E_TASK_QUEUE_FULLED,
  E_SHUTTING_DOWN,

  E_REPLICATED_START,
  E_REPLICATED_STOP,

};

#endif