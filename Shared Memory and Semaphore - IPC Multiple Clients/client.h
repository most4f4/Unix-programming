#ifndef CLIENT_H
#define CLIENT_H

// Define the permissions for the semaphore.
// SEM_PERMS allows read and write access for the user (S_IRUSR | S_IWUSR),
// and read and write access for the group (S_IRGRP | S_IWGRP).
// This combination ensures that the semaphore can be accessed by the user and the group for synchronization purposes.
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

// Semaphore name used for synchronization
// This name is used when creating or accessing the semaphore to synchronize access
// between different client processes.
const char* semName = "semName";

const char MEMNAME[]="MemDispatch";
const int BUF_LEN=1024;
const int NUM_MESSAGES=30;

struct Memory {
    int            packet_no;
    unsigned short srcClientNo;
    unsigned short destClientNo;
    char           message[BUF_LEN];
};

void *recv_func(void *arg);
#endif//CLIENT_H
