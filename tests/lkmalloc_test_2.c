#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../include/lkmalloc.h"


int main(int argc, char *argv[], char *envp[]){
    on_exit(lkreport_wrapper, NULL);
    void* testMalloc = NULL;
    void* testMalloc2 = NULL;
    void* testMalloc3 = NULL;
    void* testMalloc4 = NULL;
    lkmalloc(10, &testMalloc, 0);
    lkmalloc(100, &testMalloc2, 21);
    lkmalloc(1000, &testMalloc3, 2);
    lkmalloc(200, &testMalloc4, 6);
    void* approxMalloc = testMalloc + (sizeof(char)*8);
    lkfree(&approxMalloc, 1);
    lkfree(&testMalloc2, 0);
    lkfree(&testMalloc3, 6);
    lkfree(&testMalloc4, 0);
    return 0;
}