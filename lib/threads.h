
#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include <malloc.h>

#include "../include/error.h"
#include "logger.h"

#define MAX_THREAD 10
#define MAX_JOB 30

struct task {
  void (*job)(void*);
  void* args;
};

// The thread pool, threads in this pool scramble for jobs.
struct thread_pool {

  // Synchronizing variables
  pthread_mutex_t lock;
  pthread_cond_t cond;

  // Array of available threads
  pthread_t *threads;
  int thread_num; // threads in total
  int thread_active; // active threads

  // Use array and two pointers to represent a task queue
  struct task *tasks;
  int job_num; // size of task queue
  int job_head; // head ptr
  int job_tail; // tail ptr
  int task_left; // number of pending tasks

  char status; // 0 for running, 1 for shutting down

};

struct thread_pool* init_thread_pool(int thread_num, int job_num);
int thread_pool_add_task(struct thread_pool* pool, void (*job)(void*), void* args);
int free_thread_pool(struct thread_pool* pool);

static void* create_thread(void*);

#endif