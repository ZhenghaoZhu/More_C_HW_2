#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../include/lkmalloc.h"

/* 
 * struct for pointers that have been freed, maybe a list of linked lists 
 * ordered by size? Also include the starting and ending addresses.
 * struct for pointers that have been malloced, also the starting and ending addresses.
 * Maybe also a list of linked lists ordered by size?
 */

GHashTable* mem_node_table = NULL;
GHashTable* all_free = NULL;
GSList* memleak_ll = NULL;
GSList* bad_free_ll = NULL; 
GSList* good_free_ll = NULL; 

int lkinit(){
    mem_node_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, freeNode);
    all_free = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
    return 0;
}

int lkmalloc_def(u_int size, void **ptr, u_int flags, char* fileName, char* fxName, int lineNum){
    int regFlag = 0, initFlag = flags & LKM_INIT, overFlag = flags & LKM_OVER, underFlag = flags & LKM_UNDER, existFlag = flags & LKM_EXIST, reallocFlag = flags & LKM_REALLOC;
    if(flags == 0){
        regFlag += 1;
    } else {
        u_int tempFlags = flags ^ initFlag ^ overFlag ^ underFlag ^ existFlag ^ reallocFlag;
        if(tempFlags != 0){
            fprintf(stderr, "Invalid flag passed, please try again \n");
            *ptr = NULL;
            return -EINVAL;
        }
    }

    if(mem_node_table == NULL){
        lkinit();
    }

    if(initFlag + reallocFlag == 0){
        regFlag = 1;
    }

    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    if((ptr != NULL || *ptr != NULL) && existFlag){
        #ifdef EXTRA_CREDIT
        addFreeToList(&bad_free_ll, ptr, (char*)fileName, (char*)fxName, 0, lineNum, flags, current_time.tv_sec, current_time.tv_usec, FAILED_ALLOC);
        #endif
        fprintf(stderr, "Error: Allocating new memory with non-null pointer not possible, please try again. \n");
        return -EINVAL;
    }

    if(ptr != NULL && *ptr != NULL && !reallocFlag){
        fprintf(stderr, "Warning: Mallocing already allocated pointer, possible memory leak \n");
        addFreeToList(&memleak_ll, ptr, (char*)fileName, (char*)fxName, 0, lineNum, flags, current_time.tv_sec, current_time.tv_usec, MEM_LEAK);
        g_hash_table_remove(mem_node_table, *ptr);
        free(*ptr);
    }
    

    if(!underFlag && !overFlag){
        if(reallocFlag){
            *ptr = realloc(*ptr, size);
            if(initFlag){
                memset(*ptr, 0, size);
            }
        } else if(regFlag) {
            *ptr = malloc(size);
        } else if(initFlag) {
            *ptr = calloc(1, size);
        }
        if(*ptr != NULL){
            addNodeToTable(ptr, (char*)fxName, (char*)fileName, NORMAL_ALLOC, lineNum, size, flags, 0, current_time.tv_sec, current_time.tv_usec, false, false);
        } else {
            fprintf(stderr, "Error: There was an error trying to malloc, please try again. \n");
            return -errno;
        }
    } else if(underFlag && overFlag) {
        if(reallocFlag){
            *ptr = realloc(*ptr, size + 16);
            if(initFlag){
                memset(*ptr, 0, size + 16);
            }
        } else if(regFlag) {
            *ptr = malloc(size + 16);
        } else if(initFlag) {
            *ptr = calloc(1, size + 16);
        }
        if(*ptr != NULL){
            void* tempPtr = *ptr;
            memset(tempPtr, 0x6b, 8);
            tempPtr = *ptr + (sizeof(char)*(size + 8));
            memset(tempPtr, 0x5a, 8);
            *ptr = *ptr + (sizeof(char)*8);
            
            addNodeToTable(ptr, (char*)fxName, (char*)fileName, NORMAL_ALLOC, lineNum, size, flags, 0, current_time.tv_sec, current_time.tv_usec, true, true);
        } else {
            fprintf(stderr, "Error: There was an error trying to calloc/malloc, please try again. \n");
            return -errno;
        }
    } else if(underFlag) {
        if(reallocFlag){
            *ptr = realloc(*ptr, size + 8);
            if(initFlag){
                memset(*ptr, 0, size + 8);
            }
        } else if(regFlag) {
            *ptr = malloc(size + 8);
        } else if(initFlag) {
            *ptr = calloc(1, size + 8);
        }
        if(*ptr != NULL){
            void* tempPtr = *ptr;
            memset(tempPtr, 0x6b, 8);
            *ptr = *ptr + (sizeof(char)*8);
            addNodeToTable(ptr, (char*)fxName, (char*)fileName, NORMAL_ALLOC, lineNum, size, flags, 0, current_time.tv_sec, current_time.tv_usec, true, false);
        } else {
            fprintf(stderr, "Error: There was an error trying to calloc/malloc, please try again. \n");
            return -errno;
        }
    } else if(overFlag) {
        if(reallocFlag){
            *ptr = realloc(*ptr, size + 8);
            if(initFlag){
                memset(*ptr, 0, size + 8);
            }
        } else if(regFlag) {
            *ptr = malloc(size + 8);
        } else if(initFlag) {
            *ptr = calloc(1, size + 8);
        }
        if(*ptr != NULL){
            void* tempPtr = *ptr;
            tempPtr += (sizeof(char)*(size));
            memset(tempPtr, 0x5a, 8);
            addNodeToTable(ptr, (char*)fxName, (char*)fileName, NORMAL_ALLOC, lineNum, size, flags, 0, current_time.tv_sec, current_time.tv_usec, false, true);
        } else {
            fprintf(stderr, "Error: There was an error trying to calloc/malloc, please try again. \n");
            return -errno;
        }
    }

    if(*ptr != NULL){
        return RETURN_SUCCESS;
    } else {
        return -EINVAL;
    }
}

int lkfree_def(void **ptr, u_int flags, char* fileName, char* fxName, int lineNum, bool ending){
    int regFlag = 0, approxFlag = flags & LKF_APPROX, warnFlag = flags & LKF_WARN, unknownFlag = flags & LKF_UNKNOWN, errorFlag = flags & LKF_ERROR;

    if(flags == 0){
        regFlag += 1;
    } else {
        u_int tempFlags = flags ^ approxFlag ^ warnFlag ^ unknownFlag ^ errorFlag;
        if(tempFlags != 0){
            fprintf(stderr, "Invalid flag passed, please try again \n");
            return -EINVAL;
        }
    }

    if(mem_node_table == NULL){
        lkinit();
    }

    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    MALLOC_NODE_INFO* curPointer = (MALLOC_NODE_INFO*)g_hash_table_lookup(mem_node_table, *ptr);
    
    bool wasMiddle = false;
    void* displacedPtr = NULL;
    int diff = ptrInMiddleOfBlock(ptr);
    if(diff != -1){
        displacedPtr = *ptr - (sizeof(char)*diff);
        curPointer = (MALLOC_NODE_INFO*)g_hash_table_lookup(mem_node_table, displacedPtr);
        if(curPointer!= NULL && curPointer->underPadding){
            displacedPtr = displacedPtr - (sizeof(char)*8);
        }
        if(diff == 0){
            if(ending){
                addFreeToList(&memleak_ll, ptr, (char*)fileName, (char*)fxName, 0, lineNum, flags, current_time.tv_sec, current_time.tv_usec, MEM_LEAK);
                g_hash_table_remove(mem_node_table, *ptr);
                free(displacedPtr);
                return RETURN_SUCCESS;
            }
            addFreeToList(&good_free_ll, ptr, (char*)fileName, (char*)fxName, 0, lineNum, flags, current_time.tv_sec, current_time.tv_usec, MATCH_FREE);
            g_hash_table_remove(mem_node_table, *ptr);
            free(displacedPtr);
            return RETURN_SUCCESS;
        } else {
            addFreeToList(&bad_free_ll, ptr, (char*)fileName, (char*)fxName, 0, lineNum, flags, current_time.tv_sec, current_time.tv_usec, BAD_FREE);
            wasMiddle = true;
        }
        
    } else {
        if(g_hash_table_contains(all_free, *ptr)){
            fprintf(stderr, "Error: Double Free not allowed \n");
            addFreeToList(&bad_free_ll, &displacedPtr, (char*)fileName, (char*)fxName, 1, lineNum, flags, current_time.tv_sec, current_time.tv_usec, DOUBLE_FREE);
            return RETURN_FAILURE;
        }
    }
    
    if(regFlag || approxFlag ) {
        void* tempPtr = NULL;
        if(wasMiddle){
            tempPtr = displacedPtr;
        } else if(regFlag){
            tempPtr = *ptr;
        }
        curPointer = (MALLOC_NODE_INFO*)g_hash_table_lookup(mem_node_table, tempPtr);
        if(curPointer == NULL){
            fprintf(stderr, "Error: Tried to free pointer that was never allocated.\n");
            fprintf(stderr, "Error: Orphan free passed, pointer passed was never allocated. \n");
            addFreeToList(&bad_free_ll, &tempPtr, (char*)fileName, (char*)fxName, -EINVAL, lineNum, flags, current_time.tv_sec, current_time.tv_usec, ORPHAN_FREE);
            if(errorFlag){
                exit(EXIT_FAILURE);
            }
            return -EINVAL;
        }
        if(ending){
            addFreeToList(&memleak_ll, &tempPtr, (char*)fileName, (char*)fxName, 0, lineNum, flags, current_time.tv_sec, current_time.tv_usec, MEM_LEAK);
            g_hash_table_remove(mem_node_table, *ptr);
            free(displacedPtr);
            return RETURN_SUCCESS;
        }
        if(warnFlag && wasMiddle && approxFlag){
            fprintf(stderr, "Warning: Trying to free allocated block with pointer in middle position. \n");
        }
        if(wasMiddle){
            addFreeToList(&bad_free_ll, &tempPtr, (char*)fileName, (char*)fxName, 0, lineNum, flags, current_time.tv_sec, current_time.tv_usec, APPROX_FREE);
        }
        g_hash_table_remove(mem_node_table, tempPtr);
        free(tempPtr);

        if(errorFlag && wasMiddle && approxFlag){
            fprintf(stderr, "Exiting program because of LKF_ERROR and LKF_WARN/UNKNOWN \n");
            exit(EXIT_SUCCESS);
        }
        return RETURN_SUCCESS;
    }
    
    if(ptr == NULL || *ptr == NULL || curPointer == NULL){
        if(unknownFlag){
            fprintf(stderr, "Error: Tried to free pointer that was never allocated.\n");
            fprintf(stderr, "Error: Orphan free passed, pointer passed was never allocated. \n");
            addFreeToList(&bad_free_ll, ptr, (char*)fileName, (char*)fxName, -EINVAL, lineNum, flags, current_time.tv_sec, current_time.tv_usec, ORPHAN_FREE);
            if(errorFlag){
                exit(EXIT_FAILURE);
            }
        }
        return -EINVAL;
    }

    return -errno;
}

void lkreport_wrapper(int stat, void* args){
    lkreport(1, 63);
    return;
}

int lkreport(int fd, u_int flags){
    int noneFlag = 0, seriousFlag = flags & LKR_SERIOUS, matchFlag = flags & LKR_MATCH, badFreeFlag = flags & LKR_BAD_FREE, orphanFreeFlag = flags & LKR_ORPHAN_FREE, doubleFreeFlag = flags & LKR_DOUBLE_FREE, approxFlag = flags & LKR_APPROX;
    
    #ifdef EXTRA_CREDIT
    int failedFlag = flags & LKR_FAILED_LKMALLOC;
    #endif

    if(flags == 0){
        printf("Goes to none in lkreport \n");
        noneFlag += 1;
        lkcleanup();
        return 0;
    } else {
        u_int tempFlag = flags ^ seriousFlag ^ matchFlag ^ badFreeFlag ^ orphanFreeFlag ^ doubleFreeFlag ^ approxFlag;
        if(tempFlag != 0){
            fprintf(stderr, "Invalid flag passed, please try again \n");
            return -EINVAL;
        }
    }

    addMemTableKeys();
    char *writeBuffer = (char*)malloc(500);
    int count = 0;

    printf("\n");

    int length = sprintf(writeBuffer, "record_type, filename, fxname, line_num, timestamp, ptr_passed, retval, size_or_flags, alloc_addr_returned \n");
    if(write(fd, writeBuffer, length) == -1){
        return -errno;
    }
    memset(writeBuffer, '\0', length);
    long curTime = 0;
    if(seriousFlag){
        FREE_NODE_INFO *curFreeNode = NULL;
        size_t llSize = g_slist_length(memleak_ll);
        for(int i = 0; i < llSize; i++){
            curFreeNode = (FREE_NODE_INFO*)g_slist_nth_data(memleak_ll, i);
            length = sprintf(writeBuffer, "%i, %s, %s, %i, %ld.%ld, %p, %i, %i, %p \n", 0, curFreeNode->allocFileName, curFreeNode->allocFxName, curFreeNode->allocLineNum, curFreeNode->allocSecTime, curFreeNode->allocMsecTime, curFreeNode->allocPtrPassed, curFreeNode->allocRetVal, curFreeNode->allocSize, curFreeNode->allocPtrRet);
            if(write(fd, writeBuffer, length) == -1){
                return -errno;
            }
            memset(writeBuffer, '\0', length);
        }
    }

    if(matchFlag){
        FREE_NODE_INFO *curFreeNode = NULL;
        size_t llSize = g_slist_length(good_free_ll);
        for(int i = 0; i < llSize; i++){
            curFreeNode = (FREE_NODE_INFO*)g_slist_nth_data(good_free_ll, i);
            length = sprintf(writeBuffer, "%i, %s, %s, %i, %ld.%ld, %p, %i, %i, %p \n", 0, curFreeNode->allocFileName, curFreeNode->allocFxName, curFreeNode->allocLineNum, curFreeNode->allocSecTime, curFreeNode->allocMsecTime, curFreeNode->allocPtrPassed, curFreeNode->allocRetVal, curFreeNode->allocSize, curFreeNode->allocPtrRet);
            if(write(fd, writeBuffer, length) == -1){
                return -errno;
            }
            memset(writeBuffer, '\0', length);
            length = sprintf(writeBuffer, "%i, %s, %s, %i, %ld.%ld, %p, %i, %i, %p \n", 1, curFreeNode->freeFileName, curFreeNode->freeFxName, curFreeNode->freeLineNum, curFreeNode->freeSecTime, curFreeNode->freeMsecTime, curFreeNode->freePtrPassed, curFreeNode->freeRetVal, curFreeNode->freeFlags, NULL);
            if(write(fd, writeBuffer, length) == -1){
                return -errno;
            }
            memset(writeBuffer, '\0', length);
        }
    }


    FREE_NODE_INFO *curFreeNode = NULL;
    size_t llSize = g_slist_length(bad_free_ll);
    int curFreeType = 0x0;
    for(int i = 0; i < llSize; i++){
        curFreeNode = (FREE_NODE_INFO*)g_slist_nth_data(bad_free_ll, i);
        curFreeType = curFreeNode->freeType;
        
        if((badFreeFlag || orphanFreeFlag || doubleFreeFlag) && (curFreeType == BAD_FREE || curFreeType == ORPHAN_FREE || curFreeType == DOUBLE_FREE)){
            length = sprintf(writeBuffer, "%i, %s, %s, %i, %ld.%ld, %p, %i, %i, %p \n", 1, curFreeNode->freeFileName, curFreeNode->freeFxName, curFreeNode->freeLineNum, curFreeNode->freeSecTime, curFreeNode->freeMsecTime, curFreeNode->freePtrPassed, curFreeNode->freeRetVal, curFreeNode->freeFlags, NULL);
            if(write(fd, writeBuffer, length) == -1){
                    return -errno;
                }
            memset(writeBuffer, '\0', length);
        } else if(curFreeType == APPROX_FREE && approxFlag){
            length = sprintf(writeBuffer, "%i, %s, %s, %i, %ld.%ld, %p, %i, %i, %p \n", 0, curFreeNode->allocFileName, curFreeNode->allocFxName, curFreeNode->allocLineNum, curFreeNode->allocSecTime, curFreeNode->allocMsecTime, curFreeNode->allocPtrPassed, curFreeNode->allocRetVal, curFreeNode->allocSize, curFreeNode->allocPtrRet);
            if(write(fd, writeBuffer, length) == -1){
                return -errno;
            }
            memset(writeBuffer, '\0', length);
            length = sprintf(writeBuffer, "%i, %s, %s, %i, %ld.%ld, %p, %i, %i, %p \n", 1, curFreeNode->freeFileName, curFreeNode->freeFxName, curFreeNode->freeLineNum, curFreeNode->freeSecTime, curFreeNode->freeMsecTime, curFreeNode->freePtrPassed, curFreeNode->freeRetVal, curFreeNode->freeFlags, NULL);
            if(write(fd, writeBuffer, length) == -1){
                return -errno;
            }
            memset(writeBuffer, '\0', length);
        }

        #ifdef EXTRA_CREDIT
        if(failedFlag && curFreeType == FAILED_ALLOC){
            length = sprintf(writeBuffer, "%i, %s, %s, %i, %ld.%ld, %p, %i, %i, %p \n", 0, curFreeNode->allocFileName, curFreeNode->allocFxName, curFreeNode->allocLineNum, curFreeNode->allocSecTime, curFreeNode->allocMsecTime, curFreeNode->allocPtrPassed, curFreeNode->allocRetVal, curFreeNode->allocSize, curFreeNode->allocPtrRet);
            if(write(fd, writeBuffer, length) == -1){
                return -errno;
            }
            memset(writeBuffer, '\0', length);
        }
        #endif
        
    }
    lkcleanup();
    free(writeBuffer);
    return count;
}

void addMemTableKeys(){
    guint keyCount = 0;
    guint* keyCountP = &keyCount;
    gpointer* curKeys = g_hash_table_get_keys_as_array(mem_node_table, keyCountP);
    for(int i = 0; i < keyCount; i++){
        void* curPtrKey = (void*)curKeys[i];
        MALLOC_NODE_INFO* curNode = (MALLOC_NODE_INFO*)g_hash_table_lookup(mem_node_table, curPtrKey);
        addFreeToList(&memleak_ll, &curPtrKey, curNode->fileName, curNode->fxName, curNode->retVal, curNode->lineNum, 0, curNode->secTime, curNode->msecTime, MEM_LEAK);
    }
    g_free(curKeys);
    return;
}


int ptrInMiddleOfBlock(void** curPtr){
    if(g_hash_table_lookup(mem_node_table, *curPtr) != NULL){
        return 0;
    }
    GHashTableIter iter;
    gpointer key, value;
    void* curStart;
    void* curEnd;
    MALLOC_NODE_INFO *curNode = NULL;
    g_hash_table_iter_init(&iter, mem_node_table);
    while(g_hash_table_iter_next(&iter,  &key, &value)){
        curNode = (MALLOC_NODE_INFO*)value;
        curStart = curNode->curPtr;
        curEnd = curNode->endOfPtr;
        if(*curPtr < curEnd && *curPtr > curStart){
            return (int)(*curPtr - curStart);
        }
    }
    return -1;
}

int lkcleanup(){
    g_hash_table_destroy(mem_node_table);
    g_hash_table_destroy(all_free);
    g_slist_free_full(memleak_ll, freeNode);
    g_slist_free_full(bad_free_ll, freeNode);
    g_slist_free_full(good_free_ll, freeNode);
    return 0;
}

int addNodeToTable(void** curPtr, char* curFxName, char* curFileName, int curRecType, int curLineNum, int mallocedSize, int curSizeOrFlags, int curRetVal, long curSecs, long curMSecs, bool curUnder, bool curOver){
    MALLOC_NODE_INFO *curNode = g_new(MALLOC_NODE_INFO, 1);
    curNode->curPtr = *curPtr;
    curNode->endOfPtr = *curPtr + (sizeof(char) * mallocedSize);
    strcpy(curNode->fileName, curFileName);
    strcpy(curNode->fxName, curFxName);
    curNode->recType = curRecType;
    curNode->lineNum = curLineNum;
    curNode->size = mallocedSize;
    curNode->retVal = curRetVal;
    curNode->secTime = curSecs;
    curNode->msecTime = curMSecs;
    curNode->underPadding = curUnder;
    curNode->overPadding = curOver;
    g_hash_table_insert(mem_node_table, *curPtr, curNode);
    return 0;
}

int addFreeToList(GSList** curList, void** curPtr, char* curFileName, char* curFxName, int curRetVal, int curLineNum, int curFlags, long curSecs, long curMSecs, int curFreeType){
    MALLOC_NODE_INFO* curMallocNode = NULL;
    if(curPtr == NULL || *curPtr == NULL){
        curMallocNode = NULL;
    } else {
        curMallocNode = (MALLOC_NODE_INFO*)g_hash_table_lookup(mem_node_table, *curPtr);
    }
    FREE_NODE_INFO *curFreeNode = g_new(FREE_NODE_INFO, 1);
    curFreeNode->freePtrPassed = (curMallocNode != NULL) ? *curPtr : NULL;
    curFreeNode->allocPtrPassed = (curMallocNode != NULL) ? curMallocNode->passedPtr : NULL;
    curFreeNode->allocPtrRet =  (curMallocNode != NULL) ? curMallocNode->curPtr : NULL;
    strcpy(curFreeNode->freeFileName, curFileName);
    strcpy(curFreeNode->freeFxName, curFxName);
    if(curMallocNode != NULL){
        strcpy(curFreeNode->allocFileName, curMallocNode->fileName);
        strcpy(curFreeNode->allocFxName, curMallocNode->fxName);
    } else {
        strcpy(curFreeNode->allocFileName, "");
        strcpy(curFreeNode->allocFxName, "");
    }
    
    curFreeNode->freeRetVal = curRetVal;
    curFreeNode->allocRetVal = (curMallocNode != NULL) ? curMallocNode->retVal : -1;
    curFreeNode->freeLineNum = curLineNum;
    curFreeNode->allocLineNum =  (curMallocNode != NULL) ? curMallocNode->lineNum : -1;
    curFreeNode->allocSize = (curMallocNode != NULL) ? curMallocNode->size : -1;
    curFreeNode->freeFlags = curFlags;
    curFreeNode->freeSecTime = curSecs;
    curFreeNode->freeMsecTime = curMSecs;
    curFreeNode->allocSecTime = (curMallocNode != NULL) ? curMallocNode->secTime : 0;
    curFreeNode->allocMsecTime = (curMallocNode != NULL) ? curMallocNode->msecTime : 0;
    curFreeNode->freeType = curFreeType;
    *curList = g_slist_append(*curList, curFreeNode);
    if(curPtr != NULL && *curPtr != NULL){
        g_hash_table_insert(all_free, *curPtr, NULL);
    }
    
    return 0;
}

void freeNode(void* curNode){
    if(curNode) {
        g_free(curNode);
    }
    return;
}