#include "socket.h"
#include "http.h"

// Use functions to change these variables
static char* listen_addr = "127.0.0.1";
static int listen_port = 14514;
static char* fs_path = NULL;

static char data_buf[BUF_LEN];
static char resp_buf[4*BUF_LEN];

/*
 * Configure listening address/port and virtual file system's root
 */

// Set the listening address of the server
int set_listen_addr(char* const addr) {

  // We don't check if the address is valid here;
  // It will be checked in init_server().
  listen_addr = addr; 
  return 0;

}

// Set the listening port of the server
int set_listen_port(const int port) {
  if (port < 0 || port > 65535) {
    return -E_INVALID_PORT;
  }
  listen_port = port;
  return 0;
}

// Set the virtual file system's root of the server
int set_fs_path(char* const path) {
  if (access(path, F_OK) == -1) {
    return -E_FILE_NOT_FOUND;
  }
  fs_path = path;
  return 0;
}

/*
 * Socket lib interface
 */

int init_server() {

  // Variables
  int err = 0;
  int sock_fd, connect_fd;
  struct sockaddr_in serv_addr, cli_addr;
  socklen_t cli_addr_len = sizeof(struct sockaddr_in);
  struct http_request request;
  struct http_response response;

  // Create a ipv4 TCP socket
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    return -E_SOCK_CREATE_FAILED;
  }

  // Address of this socket
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(listen_port);

  err = inet_pton(AF_INET, listen_addr, (void *)(&serv_addr.sin_addr));
  if (err == 0) {
    return -E_INVALID_IP;
  } else if (err < 0) {
    return -E_UNRECOG;
  }

  // Bind the socket to serv_addr
  if (bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr)) == -1) {
    return -E_BIND;
  }

  // Listen
  if (listen(sock_fd, MAX_LISTEN) == -1) {
    return -E_LISTEN;
  }

  // Handle http requests
  while(1) {

    // Accept a request
    connect_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &cli_addr_len);
    if (connect_fd == -1) {
      printf("error: %s\n", strerror(errno));
      continue;
    }

    // Recieve the request post
    int data_size = recv(connect_fd, data_buf, BUF_LEN, 0);

    // Parse the request post
    memset(&request, 0, sizeof(struct http_request));
    err = parse_http_request(data_buf, &request);
    if (err < 0) {
      return err;
    }

    // Handle the request
    memset(&response, 0, sizeof(struct http_response));

    char path[64];
    strcpy(path, fs_path);
    strcat(path, request.header.url);

    handle_request(&request, &response, path);

    // Build and send the response
    build_http_response(&response, resp_buf);
    send(connect_fd, resp_buf, 4*BUF_LEN, 0);

    // Free to prevent memory leaking
    free_request(&request);
    free_response(&response);
    close(connect_fd);

  }

  close(sock_fd);

  return 0;

}