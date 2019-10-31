#include "threadpool.hpp"
#include <iostream>
#include <stdlib.h>
#include <iostream>

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
    tp->threads = new pthread_t[num];      // creates the threadpool list of size num
    tp->num_threads = num;
    tp->task_added = false;
    if((pthread_mutex_init(&(tp->thread_mutex_lock), NULL)) != 0 ){         //mutex lock initialization
        std::cerr<<"Error in initializing mutex lock\n";
    }
    if((pthread_cond_init(&(tp->thread_cond_lock), NULL)) != 0){            //conditional lock initialization
        std::cerr<<"Error in initializing mutex conditional lock\n";
    }
    
    for (int i=0; i < num; i++){
        cout << "Creating " << endl;
        pthread_create(&(tp->threads[i]), NULL,(void* (*)(void*))Thread_run, tp);           // creates num threads and inserts them in the list
    }
    return tp;
}

/**
* A C style destructor to destroy a ThreadPool object
* Parameters:
*     tp - The pointer to the ThreadPool object to be destroyed
*/
void ThreadPool_destroy(ThreadPool_t *tp){

    for(unsigned int i = 0; i < tp->num_threads;i++){
        pthread_join(tp->threads[i],NULL);
    }
    if(tp->threads){
        // free(tp->queue);

        free(tp->threads);                                                  // free the threadpool
        pthread_mutex_destroy(&(tp->thread_mutex_lock));                    // free the mutex lock
        pthread_cond_destroy(&(tp->thread_cond_lock));                      // free the conditional lock
    }
    free(tp);
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
    
    // if (pthread_mutex_lock(&(tp->thread_mutex_lock)) != 0){
    //     return false;                    // failed
        
    // }
    tp->work_queue.pq.push(ThreadPool_work_t{func,arg});                //pushing the task in the priority queue
    tp->num_tasks++;                                                    // increasing the number of tasks in the threadpool object
    //cout << (char *) tp->work_queue.pq.top().arg;
    pthread_mutex_unlock(&(tp->thread_mutex_lock));                       // unlock
    return true;                                                        // successful
}

/**
* Get a task from the given ThreadPool object
* Parameters:
*     tp - The ThreadPool object being passed
* Return:
*     ThreadPool_work_t* - The next task to run
*/
ThreadPool_work_t *ThreadPool_get_work(ThreadPool_t *tp){
    // pthread_mutex_lock(&(tp->thread_mutex_lock));                           // lock to support concurrency
    cout<<"Inside ThreadPool_get_work"<<endl;
    static ThreadPool_work_t task;                                                 // create a new task
    task.arg = (void *) tp->work_queue.pq.top().arg;                                 // assign the task.arg
    task.func = tp->work_queue.pq.top().func;                               // assign the task.func
                                                    // delete the task from the work queue
    tp->num_tasks--;                                                        // decrease number of tasks
    tp->work_queue.pq.pop();
    // pthread_mutex_unlock(&(tp->thread_mutex_lock));                           // unlock
    return &task;                                                           // return the task
}

/**
* Run the next task from the task queue
* Parameters:
*     tp - The ThreadPool Object this thread belongs to
*/
void *Thread_run(ThreadPool_t *tp){
    for(unsigned int i = 0; i < tp->num_threads;i++){
        pthread_join(tp->threads[i],NULL);
    }
    ThreadPool_t *threadPool = tp;                           // cast the pointer to threadpool_t
    while (true){                                                           // infinite loop where the threadpool checks for tasks in the work queue
        cout<<"thread runs in Thread_run before cond wait"<<endl;
        pthread_mutex_lock(&(tp->thread_mutex_lock));
        while(threadPool->task_added == false){                                     // while the work que is empty at the start the threads wait
            cout << "Waiting " << endl;
            pthread_cond_wait(&(threadPool->thread_cond_lock),&(threadPool->thread_mutex_lock));
        }
        cout << "Stopped waiting" << endl;
        ThreadPool_work_t *task;                                            // creates a new task
        cout << (char *) threadPool->work_queue.pq.top().arg << endl;
        task = ThreadPool_get_work(threadPool);                             // gets the next task
        task->func(task->arg);                                           // runs the task
        pthread_mutex_unlock(&(tp->thread_mutex_lock));                           // unlock
        if(threadPool->num_tasks == 0 ){
            break;
        }
    }
    return NULL;
}