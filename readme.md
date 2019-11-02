map_vector:
    the map vector is essentianly a vector of multimap containers which store (key,value) pairs which are both strings. it sorts the pairs automatically on insert and uses the < operator.

Time Complexities:
    MR_Emit: O(m+nlog(n))
            the inserts takes log(n) where n is the size of the container 
            and we have to iterate through the partitions.
            and m is the size of the key as in hash function we iterate through the key
    MR_GetNext: O(1)
            all the instructions are atomic

Thread Pool Library:
    Data structures:
        # Priority queue - to store the tasks in the order of the size
        # List - a list of threads

    Implementation:
        * The main function calls the ThreadPool_create() where
            - the list of pthread_t objects, num_threads, task_added, work_queue,mutex_condition and mutex_lock gets initalized
            - creates child threads equal to the number given by the the arguments and makes them run Thread_run() with the Threadpool pointer as argument
        * Each thread goes into Thread_run
            - all the threads go into an infinite while loop which only breaks if all the task are finished running
            - this is a critical section where each thread gets the work from the priority que by calling ThreadPool_get_work(). So the first statement in the while loop is the mutex lock
            - after locking, the thread checks if the task_added bool variable has benn set True by main thread or not. This variable is only set true when the tasks are added. Then the broadcasting is done to make the threads wake up from the wait get their task by calling ThreadPool_get_work.
            - WHile the threads are wiating the main thread kepps adding tasks to the priority queue using ThreadPool_add_work and after finishing adding it calls threadPool_destroy where the main thread waits for the threads to finish their work
            - ThreadPool_get_work only gets the task from the priority queue and returns them in the thread run so that the threads can run their task after unlocking the mutex lock.
            - when all the tasks are done by the threads, the main thread runs the instruction to destroy the threadpool

synchronization primitives:
    pthread_mutex_t -  a mutex lock for the thread
    pthread_cond_t - a conditional lock to make the threads wait

Testing:
    * Test cases provided by the instructor
    * GDB to fix the segmentation faults
    * Valgrind to find the memory leaks