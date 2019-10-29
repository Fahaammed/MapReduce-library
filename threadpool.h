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
    // TODO: Add other members here if needed
    bool operator<(const ThreadPool_work_t &other) const {
        struct stat st;
        struct stat st2;
        stat((const char*)arg, &st);
        stat((const char*)other.arg, &st2);
        return st.st_size < st2.st_size;
    }
} ThreadPool_work_t;

typedef struct {
    // TODO: Add members here
    // do something to compare the two files
    // source for priority queue: https://en.cppreference.com/w/cpp/container/priority_queue
    // source for stat: https://techoverflow.net/2013/08/21/how-to-get-filesize-using-stat-in-cc/
    priority_queue <ThreadPool_work_t> pq; // a priority queue where the tasks are stored.
} ThreadPool_work_queue_t;

typedef struct {
    // TODO: Add members here
    pthread_t *threads;
    ThreadPool_work_queue_t work_queue;

    pthread_attr_t scheduling_policy; // the scheduling policy of the work que
    unsigned int num_threads;  // number of threads
    pthread_mutex_t thread_mutex_lock;  // a mutex loxk for the thread
    pthread_cond_t thread_cond_lock;  // a conditional lock for the threads to wait
    unsigned int num_tasks; //number of tasks
} ThreadPool_t;


/**
* A C style constructor for creating a new ThreadPool object
* Parameters:
*     num - The number of threads to create
* Return:
*     ThreadPool_t* - The pointer to the newly created ThreadPool object
*/
ThreadPool_t *ThreadPool_create(int num);

/**
* A C style destructor to destroy a ThreadPool object
* Parameters:
*     tp - The pointer to the ThreadPool object to be destroyed
*/
void ThreadPool_destroy(ThreadPool_t *tp);

/**
* Add a task to the ThreadPool's task queue
* Parameters:
*     tp   - The ThreadPool object to add the task to
*     func - The function pointer that will be called in the thread
*     arg  - The arguments for the function
* Return:
*     true  - If successful
*     false - Otherwise
*/
bool ThreadPool_add_work(ThreadPool_t *tp, thread_func_t func, void *arg);

/**
* Get a task from the given ThreadPool object
* Parameters:
*     tp - The ThreadPool object being passed
* Return:
*     ThreadPool_work_t* - The next task to run
*/
ThreadPool_work_t *ThreadPool_get_work(ThreadPool_t *tp);

/**
* Run the next task from the task queue
* Parameters:
*     tp - The ThreadPool Object this thread belongs to
*/
void *Thread_run(ThreadPool_t *tp);


#ifdef __cplusplus
}
#endif
#endif