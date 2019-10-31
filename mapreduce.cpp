#include "mapreduce.hpp"
#include "threadpool.hpp"
#include <map>
#include <iostream>
#include <string>
#include <string.h>

using namespace std;

struct cmp_str
{
   bool operator()(char const *a, char const *b) const
   {
      return strcmp(a, b) < 0;
   }
};

typedef void (*Mapper)(char *file_name);
typedef void (*Reducer)(char *key, int partition_number);
unsigned int num_partitions; 
std::vector<std::multimap<char*, char*, cmp_str>>map_vector;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
Reducer reduce;



// creates the threadpool of num mappers
// the master thread iterates over K input data splits (represented by K files) and submits a job for each split to the thread pool
// Every time a key/value pair is generated, the Map function invokes the MR Emit library function
void MR_Run(int num_files, char *filenames[],
            Mapper map, int num_mappers,
            Reducer concate, int num_reducers){
                num_partitions = num_reducers;                                          // creating a global reference to num_reducers
                map_vector.resize(num_reducers);
                ThreadPool_t *MapperTP = ThreadPool_create(num_mappers);                // Creates the mapper threads
                for(int i = 0; i< num_files; i++){                                    
                    while(ThreadPool_add_work(MapperTP, (thread_func_t)map,filenames[i]) != true);
                }
                MapperTP->task_added = true;
                // cout << (char *) MapperTP->work_queue.pq.top().arg;
                pthread_cond_broadcast(&(MapperTP->thread_cond_lock));
                
                ThreadPool_destroy(MapperTP);

    //             std::multimap<char*,char*>::iterator iterator1;
    // // pthread_mutex_lock(&lock2);
    // // cout << "INSIDE MR PP" << endl;
    // for(iterator1 = map_vector[0].begin();iterator1 != map_vector[0].end();iterator1++){
    //     cout << "key: " << iterator1->first << ", value: " << iterator1->second << endl ;
    // }
                
                reduce = concate;                                           // destroys the mapper threads
                pthread_t ReducerTP[num_reducers];                                      // creates a reducer thread pool which is basically
                for(int i=0;i<num_reducers;i++){                                        // a list of threads
                    cout << "index : " << i << endl;
                    pthread_create(&ReducerTP[i],NULL,&wrapper_func,new int (i));
                }
                for(int i=0;i<num_reducers;i++){                                        // a list of threads
                    pthread_join(ReducerTP[i],NULL);
                }
}
//https://stackoverflow.com/questions/30234026/how-to-use-pthread-for-not-just-void-functions-with-void-argument-in-c-c
void *wrapper_func(void* num){
    int *part_num = (int*)num;
    MR_ProcessPartition(*part_num);
    delete num;
}

// *when the MR Emit function is called by a mapper thread, it first determines
//      where to write the given key/value pair by calling the MR Partition function.
// *write the intermediate key/value pair to a particular shared data structure
// *this library function must use proper synchronization primitives, such as mutex locks
// *Once the partition that a key/value pair must be written to is identified, MR Emit inserts the
//      pair in a certain position in that partition, keeping the partition sorted in ascending key order at all times.

void MR_Emit(char *key, char *value){
    cout<<"Inside MR_EMIT"<<endl;
    pthread_mutex_lock(&lock1);
    unsigned long new_hash = MR_Partition(key,num_partitions);
    map_vector[new_hash].insert(pair<char*,char*>(key,value));
    //cout << "KEY: " << map_vector[new_hash].find(key)->first << endl;
    pthread_mutex_unlock(&lock1);
}


unsigned long MR_Partition(char *key, int num_partitions){
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0'){
        hash = hash*33 + c;
    }

    return hash % num_partitions;
}

void MR_ProcessPartition(int partition_number){
    std::multimap<char*,char*>::iterator iterator1;
    pthread_mutex_lock(&lock2);
    cout << "INSIDE MR PP" << endl;
    for(iterator1 = map_vector[partition_number].begin();iterator1 != map_vector[partition_number].end();){
        iterator1 = map_vector[partition_number].begin();
        reduce(iterator1->first,partition_number);
    }
    pthread_mutex_unlock(&lock2);

}

char *MR_GetNext(char *key, int partition_number){
    std::multimap<char*,char*>::iterator iterator2;
    // cout << "INSIDE GET NEXT " << partition_number << endl;
    if(map_vector[partition_number].empty()){
        return NULL;
    }
    iterator2 = map_vector[partition_number].begin();

    if(strcmp(key,iterator2->first) == 0){
        // cout << "First : " << iterator2->first << endl;
        map_vector[partition_number].erase(iterator2);
        return key;
        
    }
    // cout << "key : " << key << " != " << iterator2->first << endl;
    return NULL;

    
}