
#include "http.h"
#include "logger.h"

// This function parses a raw http request post into a `http_request` struct.
int parse_http_request(char* raw_req, struct http_request* request) {

  // ensure that we could store the request body in `request`.
  if (request == 0) {
    return -E_INVALID_ARGS;
  }

  // parse the request method.
  size_t method_len = strcspn(raw_req, " ");

  if (memcmp(raw_req, "GET", strlen("GET")) == 0) {
    request->header.method = GET;
  } else if (memcmp(raw_req, "POST", strlen("POST")) == 0) {
    request->header.method = POST;
  } else {
    Debug("<%s:%d> Invalid post: %s\n", __FILE__, __LINE__, raw_req);
    return -E_INVALID_POST;
  }

  raw_req += method_len + 1;

  // parse the request url.
  size_t url_len = strcspn(raw_req, " ");

  request->header.url = malloc(url_len + 1);
  if (request->header.url == 0) {
    free_request(request);
    return -E_NO_MEM;
  }

  memcpy(request->header.url, raw_req, url_len);
  request->header.url[url_len] = '\0';

  raw_req += url_len + 1;

  return 0;

}

// Safely free a request struct.
int free_request(struct http_request* request) {

  free(request->header.url);
  return 0;

}

// Safely free a response struct.
int free_response(struct http_response* response) {

  free(response->content);
  return 0;

}

// status_code => status_string
const char* getStatus(int status) {
  switch (status) {
    case HTTP_OK: return "200 OK";
    case HTTP_NOT_FOUND: return "404 not found";
    case HTTP_INTERNAL_SERVER_ERROR: return "500 Internal server error";
  }
}

// Build a string-form http response out of `response` and store it in `raw_resp`
int build_http_response(struct http_response* response, char* raw_resp) {

  char len[20];
  sprintf(len, "%d", response->header.content_length);

  strcpy(raw_resp, "HTTP/1.1 ");
  strcat(raw_resp, getStatus(response->header.status));
  strcat(raw_resp, "\nContent-Type: ");
  strcat(raw_resp, response->header.content_type);
  strcat(raw_resp, "\nContent-Length: ");
  strcat(raw_resp, len);
  strcat(raw_resp, "\r\n\r\n");

  strcat(raw_resp, response->content);

  return 0;

}

// Handle a http request.
int handle_request(struct http_request* req, struct http_response* resp, char* path) {

  FILE* fd;
  long f_size;

  // Get file size and read the file in
  fd = fopen(path, "rb");
  if (fd == NULL) {
    resp->header.status = HTTP_NOT_FOUND;
    resp->header.content_type = "text/plain";
    resp->header.content_length = 0;
    // If literal "" was used here, free_request would crash.
    resp->content = (char*)malloc(1); 
    resp->content[0] = 0;
    return -E_FILE_NOT_FOUND;
  }

  fseek(fd, 0, SEEK_END);
  f_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  resp->content = (char*)malloc(f_size + 1);
  fread(resp->content, 1, f_size, fd);
  resp->content[f_size] = 0;

  fclose(fd);

  // Set the header up
  resp->header.status = HTTP_OK;
  resp->header.content_type = "text/html;charset=utf-8";
  resp->header.content_length = f_size + 1;
  
  return 0;

}