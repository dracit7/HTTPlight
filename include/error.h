
#ifndef ERROR_H
#define ERROR_H

#include <errno.h>

/*
 * Error codes
 */

enum error_codes {
  
  E_UNRECOG = 1,

  E_INVALID_ARGS,
  E_INVALID_PORT,
  E_INVALID_IP,
  E_INVALID_POST,

  E_FILE_NOT_FOUND,
  E_SOCK_CREATE_FAILED,
  E_BIND,
  E_LISTEN,
  E_NO_MEM,

};

#endif