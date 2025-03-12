#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#define SHARED_MEM_SIZE 4

int shared_fd = -1;

int32_t* counter = NULL;

sem_t* semaphore = NULL;

void init_control_mechanism(){
    semaphore = sem_open("/sem0", O_CREAT | O_EXCL, 0600, 1);
    if(semaphore == SEM_FAILED){
        fprintf(stderr,"Semaphore opening failed: %s\n",strerror(errno));
        exit(1);
    }

}

void shutdown_control_mechanism(){
    if(sem_close(semaphore)<0){
        fprintf(stderr,"Semaphore closing failed: %s\n",strerror(errno));
        exit(1);
    }
    if(sem_unlink("/sem0")<0){
        fprintf(stderr,"Unlinking failed: %s\n",strerror(errno));
        exit(1);
    }

}

void inc_counter(){
    usleep(1);
    sem_wait(semaphore);
    int32_t temp = *counter;
    usleep(1);
    temp++;
    usleep(1);
    *counter = temp;
    sem_post(semaphore);
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
