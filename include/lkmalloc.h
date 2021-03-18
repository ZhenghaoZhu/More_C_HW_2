#define RETURN_SUCCESS 0
#define RETURN_FAILURE 1

// lkmalloc flags
#define LKM_REG 0x0
#define LKM_INIT 0x1
#define LKM_OVER 0x2
#define LKM_UNDER 0x4

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

// Functions
int lkmalloc(u_int size, void **ptr, u_int flags);
int lkfree(void **ptr, u_int flags);
int lkreport(int fd, u_int flags);