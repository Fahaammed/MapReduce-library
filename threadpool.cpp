#include "threadpool.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

/**
* A C++ style constructor for creating a new ThreadPool object
* Parameters:
*     num - The number of threads to create
* Return:
*     ThreadPool_t* - The pointer to the newly created ThreadPool object
*/
ThreadPool_t *ThreadPool_create(int num){

    ThreadPool_t *tp = new ThreadPool_t;
    tp->threads = new pthread_t[num];                                                       // creates the threadpool list of size num
    tp->num_threads = num;                                                                  // initializing num_threads to num
    tp->task_added = false;                                                                 // set task_added to false so the queues can wait
    tp->work_queue = new ThreadPool_work_queue_t;                                           // initializing the work queue
    if((pthread_mutex_init(&(tp->thread_mutex_lock), NULL)) != 0 ){                         // mutex lock initialization
        std::cerr<<"Error in initializing mutex lock\n";
    }
    if((pthread_cond_init(&(tp->thread_cond_lock), NULL)) != 0){                            //conditional lock initialization
        std::cerr<<"Error in initializing mutex conditional lock\n";
    }
    for (int i=0; i < num; i++){
        pthread_create(&(tp->threads[i]), NULL,(void* (*)(void*))Thread_run, tp);           // creates num threads and inserts them in the list and makes the threads run Thread_run
    }
    return tp;                                                                              // return the threadpool
}

/**
* A C style destructor to destroy a ThreadPool object
* Parameters:
*     tp - The pointer to the ThreadPool object to be destroyed
*/
void ThreadPool_destroy(ThreadPool_t *tp){

    for(unsigned int i = 0; i < tp->num_threads;i++){                                       // join the new threads so that the main thread waits until the new threads finishes work
        pthread_join(tp->threads[i],NULL);
    }
    delete tp->work_queue;                                                                  // delete the work queue
    delete tp->threads;                                                                     // free the threadpool
    pthread_mutex_destroy(&(tp->thread_mutex_lock));                                        // free the mutex lock
    pthread_cond_destroy(&(tp->thread_cond_lock));                                          // free the conditional lock
    delete tp;                                                                              // delete the threadpool pointer
    return;
}

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
bool ThreadPool_add_work(ThreadPool_t *tp, thread_func_t func, void *arg){
    
    ThreadPool_work_t *task = new ThreadPool_work_t;                                        // create a new task object
    task->arg = arg;                                                                        // assign the argument of that task to the one given 
    task->func = func;                                                                      // assign the function of that task to the one given
    tp->work_queue->pq.push(task);                                                          // pushing the task in the priority queue
    tp->num_tasks++;                                                                        // increasing the number of tasks in the threadpool object
    return true;                                                                            // successful
}

/**
* Get a task from the given ThreadPool object
* Parameters:
*     tp - The ThreadPool object being passed
* Return:
*     ThreadPool_work_t* - The next task to run
*/
ThreadPool_work_t *ThreadPool_get_work(ThreadPool_t *tp){
    ThreadPool_work_t *task = new ThreadPool_work_t;                                        // create a new task
    task->arg = (void *) tp->work_queue->pq.top()->arg;                                     // assign the task->arg to the top task in the priority queue arg
    task->func = tp->work_queue->pq.top()->func;                                            // assign the task->func to the top task in the priority queue func                                                  
    tp->num_tasks--;                                                                        // decrease number of tasks
    tp->work_queue->pq.pop();                                                               // delete the task from the work queue
    return task;                                                                            // return the task
}

/**
* Run the next task from the task queue
* Parameters:
*     tp - The ThreadPool Object this thread belongs to
*/
void *Thread_run(ThreadPool_t *tp){
    while (true){                                                                           // infinite loop where the threadpool checks for tasks in the work queue
        pthread_mutex_lock(&(tp->thread_mutex_lock));
        while(tp->task_added == false){                                             // while task_added bool is false make the threads wait as no task was added to the queue yet
            pthread_cond_wait(&(tp->thread_cond_lock),&(tp->thread_mutex_lock));
        }
        if(tp->num_tasks == 0 ){                                                    // if all the task is the work queue is finished break out of the while loop
            break;
        }
        ThreadPool_work_t *task;                                                            // creates a new task
        task = ThreadPool_get_work(tp);                                                     // gets the next task from the work queue
        pthread_mutex_unlock(&(tp->thread_mutex_lock));                                     // unlocks the mutex lock
        task->func(task->arg);                                                              // runs the task
        
        delete task;                                                                        // deletes the task
        //pthread_mutex_unlock(&(tp->thread_mutex_lock));                                     // unlocks the mutex lock
    }
    pthread_mutex_unlock(&(tp->thread_mutex_lock));                                         // unlocks the mutex lock if the while loop broke
    return NULL;                                                                            // return NULL
}