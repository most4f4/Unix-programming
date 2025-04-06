#include <errno.h>
#include <iostream>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include "client.h"

using namespace std;
const int CLIENT_NO = 3;
bool is_running = true;

static void sigHandler(int sig)
{
	switch (sig) {
	case SIGINT:
		is_running = false;
		break;
	}
}

int main(void) {
	key_t          ShmKey;
	int            ShmID;
	struct Memory* ShmPTR;

	//Intercept ctrl-C for controlled shutdown
	struct sigaction action;
	action.sa_handler = sigHandler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGINT, &action, NULL);

	//key_t ftok(const char *pathname, int proj_id);
	//
	//The ftok() function uses the identity of the file named by the given pathname
	//and the least significant 8 bits of proj_id (which must be nonzero) to
	//generate a key_t type suitable for use with msgget(2), semget(2), or shmget(2).
	ShmKey = ftok(MEMNAME, 65);

	//int shmget(key_t key, size_t size, int shmflg);
	//
	//shmget() returns the identifier of the shared memory segment associated with
	//the value of the argument key.
	//struct Memory {
	//    int           packet_no;
	//    unsigned char sourceIP[4];
	//    unsigned char destIP[4];
	//    char          message[BUF_LEN];
	//};

	ShmID = shmget(ShmKey, sizeof(struct Memory), IPC_CREAT | 0666);
	if (ShmID < 0) {
		cout << "client3: shmget() error" << endl;
		cout << strerror(errno) << endl;
		return -1;
	}

	//void *shmat(int shmid, const void *shmaddr, int shmflg);
	//
	//shmat() attaches the shared memory segment identified by shmid to the
	//address space of the calling process. The attaching address is specified
	//by shmaddr. If shmaddr is NULL, the system chooses a suitable (unused)
	//page-aligned address to attach the segment.
	ShmPTR = (struct Memory*)shmat(ShmID, NULL, 0);
	if (ShmPTR == (void*)-1) {
		cout << "client3: shmat() error" << endl;
		cout << strerror(errno) << endl;
		return -1;
	}

	// initialize named semaphore, can be used between processes for synchronization.
	// This ensures that multiple processes can safely access shared resources without race conditions.
	sem_t* sem_id = sem_open(semName, O_CREAT, SEM_PERMS, 0);
	if (sem_id == SEM_FAILED) {
		cerr << "Client " << CLIENT_NO  << " Failed initializing semaphore: " << strerror(errno) << endl;
		exit(1);
	}

	//Client 3 starts everything
	ShmPTR->srcClientNo = CLIENT_NO;
	ShmPTR->destClientNo = 1;
	memset(ShmPTR->message, 0, BUF_LEN);
	sprintf(ShmPTR->message, "This is message 0 from client %d\n", CLIENT_NO);

	// Release the semaphore, allowing other processes to proceed.
	if (sem_post(sem_id) < 0)
		cerr << "Client " << CLIENT_NO << " Failed  releasing the semaphore: " << strerror(errno) << endl;

	for (int i = 0; i < NUM_MESSAGES && is_running; ++i) {
		if (ShmPTR->destClientNo == CLIENT_NO) {

			// Wait for the semaphore to be available before proceeding (synchronization step).
			// This ensures that only one process can access the shared memory at a time.
			if (sem_wait(sem_id) < 0)
				cerr << "Client " << CLIENT_NO << " Failed waiting for the semaphore to acquire: " << strerror(errno) << endl;

			cout << "Client " << CLIENT_NO << " has received a message from client " << ShmPTR->srcClientNo << ":" << endl;
			cout << ShmPTR->message << endl;
			//Send a message to client 1 or 2
			ShmPTR->srcClientNo = CLIENT_NO;
			ShmPTR->destClientNo = 1 + i % 2;//send a message to client 1 or 2
			memset(ShmPTR->message, 0, BUF_LEN);
			sprintf(ShmPTR->message, "This is message %d from client %d\n", i + 1, CLIENT_NO);

			// Release the semaphore so the next process can access the shared memory.
			if (sem_post(sem_id) < 0)
				cerr << "Client " << CLIENT_NO << " Failed  releasing the semaphore: " << strerror(errno) << endl;
		}
		// Sleep for 1 second to simulate a delay between message sends/receives.
		// This allows other processes to run and ensures proper synchronization between them.
		sleep(1);
	}


	// Close the reference to the semaphore once done.
	// This ensures proper cleanup and avoids resource leaks.
	if (sem_close(sem_id) != 0) {
		cerr << "Client " << CLIENT_NO << " Failed closing reference to semaphore: " << strerror(errno) << endl;
		exit(1);
	}

	// Remove the named semaphore from the system.
	// This ensures that the semaphore is cleaned up after use.
	if (sem_unlink(semName) < 0) {
		// If the semaphore removal fails, check if the error is not due to the semaphore already being removed.
		if (errno != ENOENT) {
			cerr << "Client " << CLIENT_NO << " Failed removing semaphore: " << strerror(errno) << endl;
		}
	}


	shmdt((void*)ShmPTR);
	shmctl(ShmID, IPC_RMID, NULL);

	cout << "client3: DONE" << endl;

	return 0;
}
