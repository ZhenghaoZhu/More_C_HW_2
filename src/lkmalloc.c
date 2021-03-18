#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "lkmalloc.h"

/* 
 * struct for pointers that have been freed, maybe a list of linked lists 
 * ordered by size? Also include the starting and ending addresses.
 * struct for pointers that have been malloced, also the starting and ending addresses.
 * Maybe also a list of linked lists ordered by size?
 */
int main(int argc, char *argv[], char *envp[]){
    printf("Hello World! \n");
}

int lkmalloc(u_int size, void **ptr, u_int flags){
    *ptr = malloc(size);
    if(ptr != NULL){
        return RETURN_SUCCESS;
    }
    free(ptr);
    errno = ENOMEM;
    return errno;
}

int lkfree(void **ptr, u_int flags){
    if(*ptr != NULL){
        free(*ptr);
        return RETURN_SUCCESS;
    }
    return errno;
}

int lkreport(int fd, u_int flags){
    return errno;
}

int lkFlagParser(u_int curFlags){
    return errno;
}