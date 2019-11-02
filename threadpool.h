#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <pthread.h>
#include <stdbool.h>
#include <queue>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif
using namespace std;
typedef void (*thread_func_t)(void *arg);

typedef struct ThreadPool_work_t {
    thread_func_t func;              // The function pointer
    void *arg;                       // The arguments for the function

    // source for stat: https://techoverflow.net/2013/08/21/how-to-get-filesize-using-stat-in-cc/

    bool operator<(const ThreadPool_work_t &other) const {                                                      // an operator to check form comparisons
        struct stat st;                                                                                         // creates new stat objects
        struct stat st2;
        stat((const char*)arg, &st);                                                                            // assigns those stat objects to the files to compare
        stat((const char*)other.arg, &st2);
        return st.st_size < st2.st_size;                                                                        // compares the files
    }
} ThreadPool_work_t;

typedef struct {
    // source for priority queue: https://www.geeksforgeeks.org/stl-priority-queue-for-structure-or-class/
    priority_queue <ThreadPool_work_t*> pq;                                                                     // a priority queue where the tasks are stored.
} ThreadPool_work_queue_t;

typedef struct {
    // TODO: Add members here
    pthread_t *threads;                                                                                         // a list of thread 
    ThreadPool_work_queue_t *work_queue;
    unsigned int num_threads;  // number of threads
    pthread_mutex_t thread_mutex_lock;  // a mutex loxk for the thread
    pthread_cond_t thread_cond_lock;  // a conditional lock for the threads to wait
    unsigned int num_tasks; //number of tasks
    bool task_added;
} ThreadPool_t;

ThreadPool_t *ThreadPool_create(int num);

void ThreadPool_destroy(ThreadPool_t *tp);

bool ThreadPool_add_work(ThreadPool_t *tp, thread_func_t func, void *arg);

ThreadPool_work_t *ThreadPool_get_work(ThreadPool_t *tp);

void *Thread_run(ThreadPool_t *tp);


#ifdef __cplusplus
}
#endif
#endif