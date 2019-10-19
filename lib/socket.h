
#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/error.h"

#define MAX_LISTEN 10
#define BUF_LEN 4096

/*
 * Configure listening address/port and virtual file system's root
 */

int set_listen_addr(char* const);
int set_listen_port(const int);
int set_fs_path(char* const);

/*
 * Socket lib interface
 */

int init_server(int, int, int);
int stop_server();

#endif