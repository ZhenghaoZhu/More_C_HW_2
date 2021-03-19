#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <glib.h>
#include "lkmalloc.h"

/* 
 * struct for pointers that have been freed, maybe a list of linked lists 
 * ordered by size? Also include the starting and ending addresses.
 * struct for pointers that have been malloced, also the starting and ending addresses.
 * Maybe also a list of linked lists ordered by size?
 */
int main(int argc, char *argv[], char *envp[]){
    lkmalloc(1, NULL, 1);
    lkfree(NULL, 1);
}

int lkmalloc(u_int size, void **ptr, u_int flags){
    int regFlag = 0, initFlag = flags & 0x1, overFlag = flags & 0x2, underFlag = flags & 0x4;

    if(flags == 0){
        regFlag = 1;
    }

    printf("reg: %i, init: %i, over: %i, under: %i\n", regFlag, initFlag, overFlag, underFlag);
    return 1;

    *ptr = malloc(size);
    if(ptr != NULL){
        return RETURN_SUCCESS;
    }
    free(ptr);
    errno = ENOMEM;
    return errno;
}

int lkfree(void **ptr, u_int flags){
    int regFlag = 0, approxFlag = flags & 0x1, warnFlag = flags & 0x2, unknownFlag = flags & 0x4, errorFlag = flags & 0x8;
    
    if(flags == 0){
        regFlag = 1;
    }

    printf("reg: %i, approx: %i, warn: %i, unknown: %i, error: %i \n", regFlag, approxFlag, warnFlag, unknownFlag, errorFlag);
    return 1;

    if(*ptr != NULL){
        free(*ptr);
        return RETURN_SUCCESS;
    } else {
        if(unknownFlag){
            fprintf(stderr, "Error: Tried to free NULL pointer \n");
        }
        if(errorFlag){
            exit(EXIT_FAILURE);
        }
        return errno;
    }

    return errno;
}

int lkreport(int fd, u_int flags){
    return errno;
}

int lkFlagParser(u_int curFlags){
    return errno;
}