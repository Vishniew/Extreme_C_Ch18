#ifndef COND
#define COND
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define TRUE 1
#define FALSE 0
typedef int32_t bool_t;

bool_t owner_process_set = FALSE;
bool_t owner_process = FALSE;

typedef struct{
    char *name;
    int shm_fd;
    void* map_ptr;
    char* ptr;
    size_t size;
}shared_mem_t;



shared_mem_t* shared_mem_new();
void shared_mem_delete(shared_mem_t* obj);

void shared_mem_ctor(shared_mem_t* obj, const char* name, size_t size);
void shared_mem_dtor(shared_mem_t* obj);

char* shared_mem_getptr(shared_mem_t* obj);

bool_t shared_mem_isowner(shared_mem_t* obj);
void shared_mem_setowner(shared_mem_t* obj, bool_t is_owner);



#endif
