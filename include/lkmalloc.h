#include <stdbool.h>
#include <glib.h>

#define RETURN_SUCCESS 0
#define RETURN_FAILURE 1

// lkmalloc_def flags
#define LKM_REG 0x0 //  DONE  
#define LKM_INIT 0x1 //  DONE 
#define LKM_OVER 0x2 
#define LKM_UNDER 0x4
#define LKM_EXIST 0x8 //  DONE 
#define LKM_REALLOC 0x10 //  DONE 

// lkfree flags
#define LKF_REG 0x0
#define LKF_APPROX 0x1
#define LKF_WARN 0x2
#define LKF_UNKNOWN 0x4
#define LKF_ERROR 0x8


// lkreport flags
#define LKR_NONE 0x0
#define LKR_SERIOUS 0x1
#define LKR_MATCH 0x2
#define LKR_BAD_FREE 0x4
#define LKR_ORPHAN_FREE 0x8
#define LKR_DOUBLE_FREE 0x10
#define LKR_APPROX 0x20
#define LKR_FAILED_LKMALLOC 0x40

// Malloc types
#define NORMAL_ALLOC 0x0

// Free types
#define MATCH_FREE 0x1
#define BAD_FREE    0x2
#define ORPHAN_FREE 0x3
#define DOUBLE_FREE 0x4
#define APPROX_FREE 0x5
#define MEM_LEAK 0x6

#define FAILED_ALLOC 0x7


// Lengths
#define FILENAME_LENGTH 50
#define FUNCNAME_LENGTH 50

// Functions
int lkmalloc_def(u_int size, void **ptr, u_int flags, char* fileName, char* fxName, int lineNum);
int lkfree_def(void **ptr, u_int flags, char* fileName, char* fxName, int lineNum, bool ending);
void lkreport_wrapper(int stat, void* args);
int lkreport(int fd, u_int flags);
int lkinit();
int lkcleanup();
int addNodeToTable(void** curPtr, char* curFxName, char* curFileName, int curRecType, int curLineNum, int mallocedSize, int curSizeOrFlags, int curRetVal, long curSecs, long curMSecs,  bool curUnder, bool curOver);
int ptrInMiddleOfBlock(void** curPtr);
int addFreeToList(GSList** curList, void** curPtr, char* curFileName, char* curFxName, int curRetVal, int curLineNum, int curFlags, long curSecs, long curMSecs, int curFreeType);
void freeNode(void* curNode);
void addMemTableKeys();

// Wrapper Functions
#define lkmalloc(size, ptr, flags) \
    lkmalloc_def(size, ptr, flags, (char*)__FILE__, (char*)__func__, __LINE__); \

#define lkfree(ptr, flags) \
    lkfree_def(ptr, flags, (char*)__FILE__, (char*)__func__, __LINE__, false); \
    

// Structs
typedef struct malloc_node_info {
    void* passedPtr;
    void* curPtr;
    void* endOfPtr;
    char fileName[FILENAME_LENGTH];
    char fxName[FUNCNAME_LENGTH];
    int recType;
    int lineNum;
    int size;
    int retVal;
    long secTime;
    long msecTime;
    bool underPadding;
    bool overPadding;
    
} MALLOC_NODE_INFO;

typedef struct free_node_info {
    void* freePtrPassed;
    void* allocPtrPassed;
    void* allocPtrRet;
    char freeFileName[FILENAME_LENGTH];
    char freeFxName[FUNCNAME_LENGTH];
    char allocFileName[FILENAME_LENGTH];
    char allocFxName[FUNCNAME_LENGTH];
    int freeRetVal;
    int allocRetVal;
    int freeLineNum;
    int allocLineNum;
    int allocSize;
    int freeFlags;
    long freeSecTime;
    long freeMsecTime;
    long allocSecTime;
    long allocMsecTime;
    int  freeType;
} FREE_NODE_INFO;
