#include "mapreduce.hpp"
typedef void (*Mapper)(char *file_name);
typedef void (*Reducer)(char *key, int partition_number);

void MR_Run(int num_files, char *filenames[],
            Mapper map, int num_mappers,
            Reducer concate, int num_reducers);
// creates the threadpool of num mappers
// the master thread iterates over K input data splits (represented by K files) and submits a job for each split to the thread pool
// Every time a key/value pair is generated, the Map function invokes the MR Emit library function



void MR_Emit(char *key, char *value);
// *when the MR Emit function is called by a mapper thread, it first determines
//      where to write the given key/value pair by calling the MR Partition function.
// *write the intermediate key/value pair to a particular shared data structure
// *this library function must use proper synchronization primitives, such as mutex locks
// *Once the partition that a key/value pair must be written to is identified, MR Emit inserts the
//      pair in a certain position in that partition, keeping the partition sorted in ascending key order at all times.


unsigned long MR_Partition(char *key, int num_partitions){
     unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0'){
        hash = hash*33 + c;
    }
    return hash % num_partitions;
}

void MR_ProcessPartition(int partition_number);

char *MR_GetNext(char *key, int partition_number);