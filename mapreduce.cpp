#include "mapreduce.h"
#include "threadpool.h"
#include <map>
#include <iostream>
#include <string>
#include <string.h>

using namespace std;

typedef void (*Mapper)(char *file_name);
typedef void (*Reducer)(char *key, int partition_number);
unsigned int num_partitions; 
std::vector<std::multimap<string, string> >map_vector;                                   // creates a vector of multimap which stores the key, value pairs in order
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;                                      // creates a mutex lock
Reducer reduce;


/*
* creates the threadpool of num mappers
* the master thread iterates over K input data splits (represented by num_files) and submits a job for each split to the thread pool
* then broadcasts the conditional lock and makes the bool task_added true so that the threads start working
* destroys the threadpool when the threads finish their work
* creates a list of threads called reducers threads to do the reduce
* Parameters:
*   num_files -  number of files
*   filenames[] - list of file names
*   Mapper map - the map function
*   num_mappers - number of mapper threads
*   concate - the reduce function
*   num_reducers - number of reducer threads
*/
void MR_Run(int num_files, char *filenames[],
            Mapper map, int num_mappers,
            Reducer concate, int num_reducers){
                num_partitions = num_reducers;                                                                          // creating a global reference to num_reducers
                map_vector.resize(num_reducers);                                                                        // resizing the map vector to the size equal to number of reducers
                ThreadPool_t *MapperTP = ThreadPool_create(num_mappers);                                                // Creates the mapper threads
                for(int i = 0; i< num_files; i++){                                                                      // takes each file and add them to the threadpools work queue 
                    while(ThreadPool_add_work(MapperTP, (thread_func_t)map,filenames[i]) != true);
                }
                MapperTP->task_added = true;                                                                            // after finishing adding tasks make the bool task_added to be true
                pthread_cond_broadcast(&(MapperTP->thread_cond_lock));                                                  // broadcast to the mutex_condition so that the threads start working 
                
                ThreadPool_destroy(MapperTP);                                                                           // destroy the mapper thread
                
                reduce = concate;                                                                                       // destroys the mapper threads
                pthread_t ReducerTP[num_reducers];                                                                      // creates a reducer thread pool which is basically
                for(int i=0;i<num_reducers;i++){                                                                        // a list of threads
                    pthread_create(&ReducerTP[i],NULL,&wrapper_func,new int (i));                                       
                }
                for(int i=0;i<num_reducers;i++){                                                                        // join the reducer threads 
                    pthread_join(ReducerTP[i],NULL);
                }
}

/*
* A wrapper function for the MR_ProcessPartition
* wrapper function source: https://stackoverflow.com/questions/30234026/how-to-use-pthread-for-not-just-void-functions-with-void-argument-in-c-c
* Parameters:
*   num - partition number
*/ 
void *wrapper_func(void* num){
    int *part_num = (int*)num;
    MR_ProcessPartition(*part_num);
    delete part_num;
    return(NULL);
}

/* 
* *when the MR Emit function is called by a mapper thread, it first determines
*      where to write the given key/value pair by calling the MR Partition function.
* *writes the intermediate key/value pair to the map vector 
* Parameters:
*   key - token from map
*   value - "1"
*/ 
void MR_Emit(char *key, char *value){
    
    pthread_mutex_lock(&lock1);                                                                                         // mutex_lock so that only one thread can go into the critical section  
    unsigned long new_hash = MR_Partition(key,num_partitions);                                                          // get the hash value from MR_Partion
    string skey = key;                                                                                                  // cast the key to a string
    string svalue = value;                                                                                              // cast the value to a string
    map_vector[new_hash].insert(pair<string,string>(skey,svalue));                                                      // insert the key value pair in map vector
    pthread_mutex_unlock(&lock1);                                                                                       // unlock so that other threads can access it
}
/* 
* MR_Partition is the function where the hash is done. it create a 
* unique hash for each keyand returns them
* Parameters:
*   key - token
*   num_partitions - number of partitions 
* Return:
* the hash value as an unsigned long int
*/
unsigned long MR_Partition(char *key, int num_partitions){
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0'){
        hash = hash*33 + c;
    }
    return hash % num_partitions;                                                                                       // return the hash
}
/* 
* takes the partition number and finds the task in the map_vector
* and calls reduce with that task 
* Parameters:
*   partition_number -  position in the map vector
*/
void MR_ProcessPartition(int partition_number){
    std::multimap<string,string>::iterator iterator1;                                                                   // create an iterator for the map vector
    while(!map_vector[partition_number].empty()){                                                                       // checks if map_vector with that partition number is empty. if False then continous execution
        iterator1 = map_vector[partition_number].begin();                                                               // get the key value pair with that partion number 
        reduce((char *)iterator1->first.c_str(),partition_number);                                                      // call reduce with the key from the map vector and partion number
    }
}
/* 
* takes the key and the partition number and removes that pair from the map_vector 
* and the count increases in reduce
* Parameters:
*   key - token
*   partition_number -  position in the map vector 
* Return:
*   key - token which is not NULL
*/
char *MR_GetNext(char *key, int partition_number){
    std::multimap<string,string>::iterator iterator2;                                                                   // create an iterator for the map vector
    if(map_vector[partition_number].empty()){                                                                           // checks if map_vector with that partition number is empty. if true returns NULL
        return NULL;
    }
    iterator2 = map_vector[partition_number].begin();                                                                   // get the key value pair with that partion number
    string skey = key;                                                                                                  // cast the key to a string
    if(skey == iterator2->first){                                                                                       // if the given key and the key from map vector matches then delete it and increase count
        map_vector[partition_number].erase(iterator2);
        return key;                                                                                                     // reurn the key
    }
    return NULL;                                                                                                        // otherwise return null
}