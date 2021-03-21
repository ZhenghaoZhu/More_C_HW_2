#include <stdbool.h>

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

// Malloc types
#define NORMAL_ALLOC 0x0

// Free types
#define MATCH_FREE 0x1
#define BAD_FREE 0x2
#define ORPHAN_FREE 0x3
#define DOUBLE_FREE 0x4
#define APPROX_FREE 0x5

// Lengths
#define FILENAME_LENGTH 50
#define FX_LENGTH       50

// Functions
int lkmalloc_def(u_int size, void **ptr, u_int flags, char* fileName, char* fxName, int lineNum);
int lkfree(void **ptr, u_int flags);
int lkreport(int fd, u_int flags);
int lkinit();
int lkcleanup();
int addNodeToTree(void** curPtr, char* curFxName, char* curFileName, int curRecType, int curLineNum, int mallocedSize, int curSizeOrFlags, int curRetVal, bool curUnder, bool curOver, bool curMiddle, bool curOrphan);
void freeKey(void* curKey);
void freeNode(void* curNode);
int comparePointers(void** ptr1, void** ptr2);

// Wrapper Functions
#define lkmalloc(size, ptr, flags) \
    lkmalloc_def(size, ptr, flags, (char*)__FILE__, (char*)__func__, __LINE__); \
    

// Structs
typedef struct malloc_node_info {
    void* curPtr;
    void* endOfPtr;
    char fileName[FILENAME_LENGTH];
    char fxName[FX_LENGTH];
    int recType;
    int lineNum;
    int sizeOrFlags;
    int retVal;
    double timeStamp;
    bool underPadding;
    bool overPadding;
    bool middleFree;
    bool orphanFree;
} MALLOC_NODE_INFO;

typedef struct free_node_info {
    int freedMemory;
    int freeType;
    bool approxFree;
    void* freedPtr;
} FREE_NODE_INFO;
