#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <glib.h>
#include "lkmalloc.h"

/* 
 * struct for pointers that have been freed, maybe a list of linked lists 
 * ordered by size? Also include the starting and ending addresses.
 * struct for pointers that have been malloced, also the starting and ending addresses.
 * Maybe also a list of linked lists ordered by size?
 */

GHashTable* mem_node_table = NULL;
GSList* free_node_ll = NULL; 


int main(int argc, char *argv[], char *envp[]){
    void* testMalloc;
    // void* testMalloc2;
    lkmalloc(10, &testMalloc, 2);
    MALLOC_NODE_INFO* curPointer = (MALLOC_NODE_INFO*)g_hash_table_lookup(mem_node_table, testMalloc);
    if(curPointer != NULL){
        printf("%i \n", curPointer->underPadding);
    } else {
        printf("Sadness \n");
    }
    free(testMalloc);
    lkcleanup();
    return 0;
}

int lkinit(){
    mem_node_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, freeNode);
    return 0;
}

int lkmalloc_def(u_int size, void **ptr, u_int flags, char* fileName, char* fxName, int lineNum){
    int regFlag = 0, initFlag = flags & LKM_INIT, overFlag = flags & LKM_OVER, underFlag = flags & LKM_UNDER, existFlag = flags & LKM_EXIST, reallocFlag = flags & LKM_REALLOC;
    // printf("flags: %i reg: %i, init: %i, over: %i, under: %i, exist: %i, realloc: %i \n", flags, regFlag, initFlag, overFlag, underFlag, existFlag, reallocFlag);
    if(flags == 0){
        regFlag += 1;
    } else {
        flags = flags ^ initFlag ^ overFlag ^ underFlag ^ existFlag ^ reallocFlag;
        if(flags != 0){
            fprintf(stderr, "Invalid flag passed, please try again \n");
            *ptr = NULL;
            return -EINVAL;
        }
    }

    if(mem_node_table == NULL){
        lkinit();
    }

    if((ptr != NULL || *ptr != NULL) && existFlag){
        fprintf(stderr, "Error: Allocating new memory with non-null pointer not possible, please try again. \n");
        return -EINVAL;
    }

    if(reallocFlag){
        // TODO  Need to add redzones if statements
        if(*ptr != NULL){
            fprintf(stderr, "Warning: Mallocing already allocated pointer, possible memory leak \n");
            void* tempPtr = realloc(*ptr, size);
            void* tempPtrPtr = &tempPtr;
            if(g_hash_table_lookup(mem_node_table, *ptr) && tempPtr != NULL){
                // g_hash_table_remove(mem_node_table, *ptr);
                addNodeToTree(tempPtrPtr, (char*)fxName, (char*)fileName, NORMAL_ALLOC, lineNum, size, flags, 0, false, false, false, false);
                return RETURN_SUCCESS;
            } else {
                fprintf(stderr, "Error: There was an error trying to realloc, please try again. \n");
                return -errno;
            }
        } else {
            fprintf(stderr, "Error: You tried to realloc a null pointer, please try again. \n");
            return -EINVAL;
        }
    }

    if(!underFlag && !overFlag){
        if(initFlag){
            *ptr = calloc(1, size);
            if(*ptr != NULL){
                addNodeToTree(ptr, (char*)fxName, (char*)fileName, NORMAL_ALLOC, lineNum, size, flags, 0, false, false, false, false);
            } else {
                fprintf(stderr, "Error: There was an error trying to calloc, please try again. \n");
                return -errno;
            }
        } else {
            *ptr = malloc(size);
            if(*ptr != NULL){
                addNodeToTree(ptr, (char*)fxName, (char*)fileName, NORMAL_ALLOC, lineNum, size, flags, 0, false, false, false, false);
            } else {
                fprintf(stderr, "Error: There was an error trying to malloc, please try again. \n");
                return -errno;
            }
        }
    } else if(underFlag && overFlag) {
        if(initFlag){
            *ptr = calloc(1, size + 16);
        } else {
            *ptr = malloc(size + 16);
        }
        if(*ptr != NULL){
            void* tempPtr = *ptr;
            memset(tempPtr, 0x6b, 8);
            tempPtr = *ptr + (sizeof(char)*(size + 8));
            memset(tempPtr, 0x5a, 8);
            *ptr = *ptr + (sizeof(char)*8);
            addNodeToTree(ptr, (char*)fxName, (char*)fileName, NORMAL_ALLOC, lineNum, size, flags, 0, true, true, false, false);
        } else {
            fprintf(stderr, "Error: There was an error trying to calloc/malloc, please try again. \n");
            return -errno;
        }
    } else if(underFlag) {
        if(initFlag){
            *ptr = calloc(1, size + 8);
        } else {
            *ptr = malloc(size + 8);
        }
        if(*ptr != NULL){
            void* tempPtr = *ptr;
            memset(tempPtr, 0x6b, 8);
            *ptr = *ptr + (sizeof(char)*8);
            addNodeToTree(ptr, (char*)fxName, (char*)fileName, NORMAL_ALLOC, lineNum, size, flags, 0, true, false, false, false);
        } else {
            fprintf(stderr, "Error: There was an error trying to calloc/malloc, please try again. \n");
            return -errno;
        }
    } else if(overFlag) {
        if(initFlag){
            *ptr = calloc(1, size + 8);
        } else {
            *ptr = malloc(size + 8);
        }
        if(*ptr != NULL){
            void* tempPtr = *ptr;
            tempPtr += (sizeof(char)*(size));
            memset(tempPtr, 0x5a, 8);
            addNodeToTree(ptr, (char*)fxName, (char*)fileName, NORMAL_ALLOC, lineNum, size, flags, 0, false, true, false, false);
        } else {
            fprintf(stderr, "Error: There was an error trying to calloc/malloc, please try again. \n");
            return -errno;
        }
    }

    if(*ptr != NULL){
        // fprintf(stdout, "Success mallocing %p \n", *ptr);
        return RETURN_SUCCESS;
    }
    return -EINVAL;
}

int lkfree(void **ptr, u_int flags){
    int regFlag = 0, approxFlag = flags & LKF_APPROX, warnFlag = flags & LKF_WARN, unknownFlag = flags & LKF_UNKNOWN, errorFlag = flags & LKF_ERROR;
    // printf("reg: %i, approx: %i, warn: %i, unknown: %i, error: %i \n", regFlag, approxFlag, warnFlag, unknownFlag, errorFlag);

    if(flags == 0){
        regFlag += 1;
    } else {
        flags = flags ^ approxFlag ^ warnFlag ^ unknownFlag ^ errorFlag;
        if(flags != 0){
            fprintf(stderr, "Invalid flag passed, please try again \n");
            return -EINVAL;
        }
    }

    if(ptr == NULL || *ptr == NULL){
        fprintf(stderr, "Unable to free pointer, please try again. \n");
        if(unknownFlag){
            fprintf(stderr, "Error: Tried to free NULL pointer \n");
            if(errorFlag){
                exit(EXIT_FAILURE);
            }
        }
        
        return -EINVAL;
    }

    // TODO  Search for given pointer in tree, if not found then UNKNOWN_FLAG

    if(approxFlag){
        if(warnFlag){
            if(errorFlag){
                fprintf(stderr, "Conditions of passed pointer matched LKF_WARN \n");
                exit(EXIT_FAILURE);
            }
        }
    } else if(regFlag) {
        free(*ptr);
        return RETURN_SUCCESS;
    }

    return -errno;
}

int lkreport(int fd, u_int flags){
    int noneFlag = 0, seriousFlag = flags & LKR_SERIOUS, matchFlag = flags & LKR_MATCH, badFreeFlag = flags & LKR_BAD_FREE, orphanFreeFlag = flags & LKR_ORPHAN_FREE, doubleFreeFlag = flags & LKR_DOUBLE_FREE, approxFlag = flags & LKR_APPROX;
    if(flags == 0){
        noneFlag += 1;
        return RETURN_SUCCESS;
    } else {
        flags = flags ^ seriousFlag ^ matchFlag ^ badFreeFlag ^ orphanFreeFlag ^ doubleFreeFlag ^ approxFlag;
        if(flags != 0){
            fprintf(stderr, "Invalid flag passed, please try again \n");
            return -EINVAL;
        }
    }
    return errno;
}

int lkFlagParser(u_int curFlags){
    return errno;
}

int lkcleanup(){
    g_hash_table_destroy(mem_node_table);
    return 0;
}

int addNodeToTree(void** curPtr, char* curFxName, char* curFileName, int curRecType, int curLineNum, int mallocedSize, int curSizeOrFlags, int curRetVal, bool curUnder, bool curOver, bool curMiddle, bool curOrphan){
    MALLOC_NODE_INFO *curNode = g_new(MALLOC_NODE_INFO, 1);
    curNode->curPtr = *curPtr;
    curNode->endOfPtr = *curPtr + (sizeof(char) * mallocedSize);
    strcpy(curNode->fileName, curFileName);
    strcpy(curNode->fxName, curFxName);
    curNode->recType = curRecType;
    curNode->lineNum = curLineNum;
    if(curRecType == NORMAL_ALLOC){
        curNode->sizeOrFlags = mallocedSize;
    } else {
        curNode->sizeOrFlags = curSizeOrFlags;
    }
    curNode->retVal = curRetVal;
    curNode->timeStamp = 0.0;
    curNode->underPadding = curUnder;
    curNode->overPadding = curOver;
    curNode->middleFree = curMiddle;
    curNode->orphanFree = curOrphan;
    g_hash_table_insert(mem_node_table, *curPtr, curNode);
    return 0;
}

void freeKey(void* curKey){
    if(curKey) {
        g_free(curKey);
    }
    return;
}

void freeNode(void* curNode){
    MALLOC_NODE_INFO *curPointer = (MALLOC_NODE_INFO*) curNode;
    // if(curPointer->curPtr){
    //     free(curPointer->curPtr);
    // }
    if(curNode) {
        g_free(curNode);
    }
    return;
}

int comparePointers(void** ptr1, void** ptr2){
    if(*ptr1 > *ptr2){
        return 1;
    } else if(*ptr1 == *ptr2){
        return 0;
    } else {
        return -1;
    }
}