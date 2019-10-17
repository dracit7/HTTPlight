
#ifndef HTTP_H
#define HTTP_H

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "../include/error.h"

enum HTTP_METHOD {
  GET = 1,
  POST,
};

enum HTTP_STATUS {
  HTTP_OK = 200,
  HTTP_NOT_FOUND = 404,
  HTTP_INTERNAL_SERVER_ERROR = 500,
};

struct http_request_header {
  char method;
  char* url;
};

struct http_response_header {
  int status;
  char* content_type;
  int content_length;
};

struct http_request {
  struct http_request_header header;
  char* content;
};

struct http_response {
  struct http_response_header header;
  char* content;
};

int parse_http_request(char*, struct http_request*);
int handle_request(struct http_request*, struct http_response*, char*);
int build_http_response(struct http_response*, char*);

int free_request(struct http_request*);
int free_response(struct http_response*);

const char* getStatus(int);

#endif