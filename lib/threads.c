
#include "threads.h"

// Initialize a thread pool.
struct thread_pool* init_thread_pool(int thread_num, int job_num) {

  if (thread_num < 0 || thread_num > MAX_THREAD || job_num < 0 || job_num > MAX_JOB) {
    return NULL;
  }

  // Allocate an empty thread pool
  struct thread_pool* pool = (struct thread_pool*)malloc(sizeof(struct thread_pool));
  if (pool == NULL) {
    return NULL;
  }

  // Initialize the thread pool
  pool->job_num = job_num;
  pool->thread_num = pool->thread_active = 0;
  pool->job_head = pool->job_tail = pool->task_left = 0;
  pool->status = 1;

  if (pthread_mutex_init(&pool->lock, NULL) != 0) {
    free(pool);
    return NULL;
  }

  if (pthread_cond_init(&(pool->cond), NULL) != 0) {
    free(pool);
    return NULL;
  }

  pool->threads = (pthread_t*)malloc(thread_num*sizeof(pthread_t));
  pool->tasks = (struct task*)malloc(job_num*sizeof(struct task));
  if (!(pool->threads && pool->tasks)) {
    free(pool);
    return NULL;
  }

  // Create worker threads
  for (int i = 0; i < thread_num; i++) {
    if (pthread_create(&(pool->threads[i]), NULL, create_thread, (void*)pool) != 0) {
      free_thread_pool(pool);
      return NULL;
    }
    pool->thread_num++;
    pool->thread_active++;
  }

  pool->status = 0;
  return pool;

}

// Add a task into thread pool's task queue.
int thread_pool_add_task(struct thread_pool* pool, void (*job)(void*), void* args) {

  int next_task_i;

  // Check arguements
  if (pool == NULL || job == NULL) {
    return -E_INVALID_ARGS;
  }

  // Lock pool before operating on it
  if(pthread_mutex_lock(&(pool->lock)) != 0) {
    return -E_LOCK_FAILED;
  }

  // Find the next empty job slot
  next_task_i = (pool->job_tail + 1) % pool->job_num;

  // Check if the task queue is full
  if (pool->task_left == pool->job_num) {
    pthread_mutex_unlock(&pool->lock);
    return -E_TASK_QUEUE_FULLED;
  }

  // Check if the thread pool is shutting down
  if (pool->status == 1) {
    pthread_mutex_unlock(&pool->lock);
    return -E_SHUTTING_DOWN;
  }

  // Add task to queue
  pool->tasks[pool->job_tail].job = job;
  pool->tasks[pool->job_tail].args = args;
  pool->job_tail = next_task_i;
  pool->task_left += 1;

  // Make a broadcast to all threads that a new task has enrolled.
  if (pthread_cond_signal(&(pool->cond)) != 0) {
    pthread_mutex_unlock(&pool->lock);
    return -E_LOCK_FAILED;
  }

  // Release the lock before returning
  pthread_mutex_unlock(&pool->lock);

  return 0;

}

int free_thread_pool(struct thread_pool *pool) {

  int err = 0;

  if (pool == NULL) {
    return -E_INVALID_ARGS;
  }

  // Lock pool before operating on it
  if (pthread_mutex_lock(&(pool->lock)) != 0) {
    return -E_LOCK_FAILED;
  }

  // Prevent replicated shutdown
  if (pool->status == 1) {
    pthread_mutex_unlock(&(pool->lock));
    return -E_SHUTTING_DOWN;
  }

  pool->status = 1;

  // Notify all threads we're shutting down
  if (pthread_cond_broadcast(&(pool->cond)) != 0) {
    pthread_mutex_unlock(&(pool->lock));
    return -E_LOCK_FAILED;
  }

  pthread_mutex_unlock(&(pool->lock));

  // Wait until all threads are stopped
  for(int i = 0; i < pool->thread_num; i++) {
    if (pthread_join(pool->threads[i], NULL) != 0) {
      continue;
    }
  }

  // Free all allocated memory
  free(pool->threads);
  free(pool->tasks);

  pthread_mutex_lock(&(pool->lock));
  pthread_mutex_destroy(&(pool->lock));
  pthread_cond_destroy(&(pool->cond));

  free(pool);

  return 0;

}

static void* create_thread(void* _pool) {

  struct thread_pool* pool = (struct thread_pool*)_pool;
  struct task job;

  while (1) {
 
    // Acquire the lock, then use `pthread_cond_wait` to release it and wait
    // for a signal.
    pthread_mutex_lock(&(pool->lock));
    while ((pool->task_left == 0) && (pool->status == 0)) {
      pthread_cond_wait(&(pool->cond), &(pool->lock));
    }

    // Stop this thread if shutting down
    if (pool->status == 1) {
      break;
    }

    // Get the job
    job.job = pool->tasks[pool->job_head].job;
    job.args = pool->tasks[pool->job_head].args;

    pool->job_head = (pool->job_head + 1) % pool->job_num;
    pool->task_left--;

    // Unlock
    pthread_mutex_unlock(&(pool->lock));

    // Do the job
    (*(job.job))(job.args);

  }

  pool->thread_active--;

  pthread_mutex_unlock(&(pool->lock));
  pthread_exit(NULL);

  return NULL;

}