#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>

#define SHARED_MEM_SIZE 4

int shared_fd = -1;
int mutex_shm_fd =  -1;

int32_t *counter = NULL;

pthread_mutex_t *mutex = NULL;


void init_control_mechanism(){
    mutex_shm_fd = shm_open("/mutex0", O_CREAT | O_RDWR, 0600);

    if(mutex_shm_fd < 0){
        fprintf(stderr, "ERROR: Failed to create shared memory: %s\n", strerror(errno));
        exit(1);
    }

    if(ftruncate(mutex_shm_fd, sizeof(pthread_mutex_t))<0){
        fprintf(stderr, "ERROR: Truncation of mutex failed: %s\n", strerror(errno));
        exit(1);

    }

    void *map = mmap(0, sizeof(pthread_mutex_t),PROT_READ | PROT_WRITE, MAP_SHARED, mutex_shm_fd, 0);

    if (map == MAP_FAILED){
        fprintf(stderr, "ERROR: Mapping failed: %s\n", strerror(errno));
        exit(1);

    }

    mutex = (pthread_mutex_t *)map;

    int ret = -1;

    pthread_mutexattr_t attr;

    if((ret = pthread_mutexattr_init(&attr))){

        fprintf(stderr, "ERROR: Failed to init attr: %s\n", strerror(errno));
        exit(1);
    }

    if((ret = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)))
    {   
        fprintf(stderr, "ERROR: Failed to set mutex attr: %s\n", strerror(errno));
        exit(1);
    }

    if((ret = pthread_mutex_init(mutex, &attr))){
        fprintf(stderr, "ERROR: Failed to init mutex : %s\n", strerror(errno));
        exit(1);

    }
    if((ret = pthread_mutexattr_destroy(&attr))){
        fprintf(stderr, "ERROR: Failed to destroy mutex attr: %s\n", strerror(errno));
        exit(1);
    }

}

void shutdown_control_mechanism(){
    int ret = -1;

    if(ret = pthread_mutex_destroy(mutex))
    {
        fprintf(stderr, "ERROR: Failed to destroy mutex : %s\n", strerror(errno));
        exit(1);
    }
    if(munmap(mutex, sizeof(pthread_mutex_t)))
    {
        fprintf(stderr, "ERROR: Failed to unmap the mutex : %s\n", strerror(errno));
        exit(1);
    }
    if(close(mutex_shm_fd) < 0)
    {
        fprintf(stderr, "ERROR: Failed to close mutex : %s\n", strerror(errno));
        exit(1);
    }
    if(shm_unlink("/mutex0") < 0)
    {
        fprintf(stderr, "ERROR: Failed to unlink mutex : %s\n", strerror(errno));
        exit(1);
    }

}
void inc_counter(){
    usleep(1);
    pthread_mutex_lock(mutex);
    int32_t temp = *counter;
    usleep(1);
    temp++;
    usleep(1);
    *counter = temp;
    pthread_mutex_unlock(mutex);
    usleep(1);
}

void init_shared(){
    shared_fd = shm_open("/shm0", O_CREAT | O_RDWR, 0600);

}
void shutdown_shared(){
    shm_unlink("/shm0");
}


int main(int argc, char **argv) {
    init_shared();
    init_control_mechanism();
    
    if( ftruncate( shared_fd, SHARED_MEM_SIZE * sizeof(char)) < 0){
        fprintf(stderr, "Trucation failed : %s\n", strerror(errno));
        exit(1);
    }
    
    void *map = mmap(0, SHARED_MEM_SIZE, PROT_WRITE, MAP_SHARED, shared_fd, 0);

    counter = (int *)map;
    *counter = 0;
    
    pid_t pid = fork();

    if(pid)
    {
        inc_counter();
        
        fprintf(stdout,"Parent sees the counter as %d\n", *counter);

        int status = -1;
        wait(&status);
        fprintf(stdout, "Child process finished with status: %d\n",status);


    }

    else{
        inc_counter();
        
        fprintf(stdout,"Child sees the counter as %d\n", *counter);
        
    }

    
    munmap(counter, SHARED_MEM_SIZE); 
    close(shared_fd);

    if(pid)
    {
        shutdown_shared();
        shutdown_control_mechanism();
    }

    return 0;
}
