#include "socket.h"
#include "http.h"
#include "logger.h"
#include "threads.h"

// Use functions to change these variables
static char* listen_addr = "127.0.0.1";
static int listen_port = 14514;
static char* fs_path = NULL;

static char server_status = 1;
pthread_mutex_t status_lock;

static void handle_connect(void* _connect_fd);

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

// Stop the server.
int stop_server() {

  int sock_fd;
  struct sockaddr_in addr = {0};

  Log("===== Stopping the server... =====\n");

  // Set server's status to stopping
  if (pthread_mutex_lock(&status_lock) < 0) {
    return -E_LOCK_FAILED;
  }
  if (server_status == 1) {
    Error("The server has not started!\n");
    pthread_mutex_unlock(&status_lock);
    return -E_REPLICATED_STOP;
  }
  server_status = 1;
  pthread_mutex_unlock(&status_lock);

  // Send a request to server to shut it down
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    return -E_SOCK_CREATE_FAILED;
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(listen_addr); 
  addr.sin_port = htons(listen_port);

  if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) { 
    return -E_CONNECT;
  } 

  close(sock_fd);

  return 0;

}

/*
 * Socket lib interface
 */

// Init the server.
// - thread_num: max threads of the server
// - job_num: max connections handled concurrently by the server
// - timeout: timeout by second
int init_server(int thread_num, int job_num, int timeout) {

  // Variables
  int err = 0;
  int sock_fd, connect_fd;
  struct sockaddr_in serv_addr, cli_addr;
  socklen_t cli_addr_len = sizeof(struct sockaddr_in);

  // If server has started
  if (server_status == 0) {
    Error("The server has already started!\n");
    return -E_REPLICATED_START;
  }

  Log("===== Starting the server... =====\n");

  struct thread_pool* pool = init_thread_pool(thread_num, job_num);
  pthread_mutex_init(&status_lock, NULL);
  server_status = 0;

  Log("Creating the thread pool...\n");

  // Create a ipv4 TCP socket
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    server_status = 1;
    return -E_SOCK_CREATE_FAILED;
  }

  Log("Creating the server socket...\n");

  // Address of this socket
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(listen_port);

  err = inet_pton(AF_INET, listen_addr, (void *)(&serv_addr.sin_addr));
  if (err == 0) {
    close(sock_fd);
    server_status = 1;
    return -E_INVALID_IP;
  } else if (err < 0) {
    close(sock_fd);
    server_status = 1;
    return -E_UNRECOG;
  }

  Log("Loading the server's address...\n");

  // Bind the socket to serv_addr
  if (bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr)) == -1) {
    close(sock_fd);
    server_status = 1;
    return -E_BIND;
  }

  Log("Binding...\n");

  // Listen
  if (listen(sock_fd, MAX_LISTEN) == -1) {
    close(sock_fd);
    server_status = 1;
    return -E_LISTEN;
  }

  Log("===== Server started =====\n");
  Log("Listening to requests...\n");

  // Handle http requests
  while(1) {

    // Check the server status
    if (pthread_mutex_lock(&status_lock) < 0) {
      server_status = 1;
      return -E_LOCK_FAILED;
    }
    if (server_status == 1) {
      pthread_mutex_unlock(&status_lock);
      break;
    }
    pthread_mutex_unlock(&status_lock);

    // Accept a request
    connect_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &cli_addr_len);
    if (connect_fd == -1) {
      printf("error: %s\n", strerror(errno));
      continue;
    }

    Log("=== Accepted a valid client: <%d> ===\n", connect_fd);
    Log("<%d> IP: %s\n", connect_fd, inet_ntoa(cli_addr.sin_addr));
    Log("<%d> Port: %u\n", connect_fd, cli_addr.sin_port);

    err = thread_pool_add_task(pool, handle_connect, (void *)(long)(connect_fd));
    if (err < 0) {
      Error("<%s:%d> Error code %d\n", __FILE__, __LINE__, -err);
      server_status = 1;
      return err;
    }
    Log("<%d> Registered client's task.\n", connect_fd);

  }

  free_thread_pool(pool);
  close(sock_fd);

  Log("===== Server stopped =====\n");

  return 0;

}

// The task to execute in a thread.
void handle_connect(void* _connect_fd) {

  char data_buf[BUF_LEN] = {0};
  char resp_buf[1024*BUF_LEN];
  int connect_fd = (int)(long)_connect_fd;

  int err;
  struct http_request request;
  struct http_response response;

  Log("<%d> Task scheduled.\n", connect_fd);

  // Recieve the request post
  int data_size = recv(connect_fd, data_buf, BUF_LEN, 0);
  if (err == -1) {
    Error("<%s:%d> recv() failed.\n", __FILE__, __LINE__);
    Log("=== Closed connection to client <%d>. ===\n", connect_fd);
    close(connect_fd);
    return;
  }

  // Parse the request post
  memset(&request, 0, sizeof(struct http_request));
  err = parse_http_request(data_buf, &request);
  if (err < 0) {
    Error("<%s:%d> Error code %d\n", __FILE__, __LINE__, -err);
    Log("=== Closed connection to client <%d>. ===\n", connect_fd);
    close(connect_fd);
    return;
  }

  Log("<%d> Request checked: HTTP %s request.\n", connect_fd,
    (request.header.method == POST) ? "POST" : "GET");
  Log("<%d> Request url: %s\n", connect_fd, request.header.url);

  // Handle the request
  memset(&response, 0, sizeof(struct http_response));

  char path[64];
  strcpy(path, fs_path);
  strcat(path, request.header.url);

  err = handle_request(&request, &response, path);
  if (err < 0 && err != -E_FILE_NOT_FOUND) {
    Error("<%s:%d> Error code %d\n", __FILE__, __LINE__, -err);
    Log("=== Closed connection to client <%d>. ===\n", connect_fd);
    close(connect_fd);
    return;
  }

  // Build and send the response
  build_http_response(&response, resp_buf);

  // Do not use strcat here, or binary file would highly likely be cutted.
  size_t header_len = strlen(resp_buf);
  memmove(resp_buf+header_len, response.content, response.header.content_length);

  size_t sent_len = send(connect_fd, resp_buf, response.header.content_length + header_len, 0);
  
  Log(
    "<%d> Sent response to client: %s, %d bytes in total, %d in content\n", 
    connect_fd,
    getStatus(response.header.status),
    sent_len,
    sent_len - header_len
  );

  // Free to prevent memory leaking
  free_request(&request);
  free_response(&response);
  close(connect_fd);

  Log("=== Closed connection to client <%d>. ===\n", connect_fd);

}