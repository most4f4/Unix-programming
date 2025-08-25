#include <errno.h> 
#include <iostream> 
#include <queue> 
#include <signal.h> 
#include <string.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <unistd.h>
#include "client.h"


using namespace std;

key_t key;
int msgid;
bool is_running;
queue<Message> message;

/* shared mutex between receive thread and send */
pthread_mutex_t lock_x;

void* recv_func(void* arg);

static void shutdownHandler(int sig)
{
	switch (sig) {
	case SIGINT:
		cout << "\nCrtl + C Shutting Down" << endl;
		is_running = false;
		break;
	}
}

int main()
{
	pthread_t tid_r, tid_w;

	//Configue and set signal handler for SIGINT
	struct sigaction action;
	action.sa_handler = shutdownHandler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGINT, &action, NULL);

	// Generate unique key for message queue
	key = ftok("serverclient", 65);
	if (key == -1) {
		cout << "Error generating key: " << strerror(errno) << endl;
		return -1;
	}

	// Create the message queue
	msgid = msgget(key, 0666 | IPC_CREAT);
	if (msgid == -1) {
		cout << "Error creating message queue: " << strerror(errno) << endl;
		return -1;
	}

	// Initializes a mutex (lock)
	if (pthread_mutex_init(&lock_x, NULL) != 0) {
		cout << "Error initializing mutex: " << strerror(errno) << endl;
		return -1;
	}

	is_running = true;

	// Creates a new thread that runs the function recv_func()
	if (pthread_create(&tid_r, NULL, recv_func, NULL) != 0) {
		is_running = false;
		cout << "Error creating receive thread: " << strerror(errno) << endl;
		return -1;
	}

	while (is_running) {
		if (message.size() > 0) {
			pthread_mutex_lock(&lock_x);
			Message sendMsg = message.front();
			message.pop();
			pthread_mutex_unlock(&lock_x);

			sendMsg.mtype = sendMsg.msgBuf.dest;
			if (msgsnd(msgid, &sendMsg, sizeof(sendMsg), 0) == -1) {
				cout << "Error sending message: " << strerror(errno) << endl;
			}
			else {
				cout << "Server dispatched a message from client "
					<< sendMsg.msgBuf.source << " to --> client "
					<< sendMsg.msgBuf.dest << " : " << sendMsg.msgBuf.buf << endl;
			}
		}
		else {
			usleep(1000); // Sleep briefly to avoid busy-waiting
		}
	}

	// Send "Quit" messages to all clients on shutdown
	Message quitMsg;
	sprintf(quitMsg.msgBuf.buf, "Quit");

	for (int i = 0; i < 3; i++) {
		quitMsg.mtype = i + 1;
		quitMsg.msgBuf.source = 0;
		quitMsg.msgBuf.dest = i + 1;
		if (msgsnd(msgid, &quitMsg, sizeof(quitMsg), 0) == -1) {
			cout << "Error sending Quit to client " << i + 1 << ": " << strerror(errno) << endl;
		}
		else {
			cout << "Server sent Quit message to client " << i + 1 << endl;
		}

	}

	// Wait for receive thread to exit
	if (pthread_join(tid_r, NULL) != 0) {
		cout << "Error joining receive thread: " << strerror(errno) << endl;
		return -1;
	}

	cout << "Server: quitting ..." << endl;
	msgctl(msgid, IPC_RMID, NULL);
	return 0;
}

void* recv_func(void* arg) {
	while (is_running) {
		Message msg;
		// Use IPC_NOWAIT with msgrcv to make it non-blocking, allowing the loop to check is_running regularly.
		if (msgrcv(msgid, &msg, sizeof(msg), 4, IPC_NOWAIT) == -1) {
			if (errno == ENOMSG) { // No message available
				usleep(1000); // Brief sleep to avoid busy-waiting
				continue;
			}
			if (errno == EINTR || errno == EIDRM) { // Signal or queue removed
				break;
			}
			cout << "Error receiving message: " << strerror(errno) << endl;
			continue;
		}
		pthread_mutex_lock(&lock_x);
		message.push(msg);
		cout << "Server received a message from client " << msg.msgBuf.source
			<< " to --> client " << msg.msgBuf.dest << " : " << msg.msgBuf.buf << endl;
		pthread_mutex_unlock(&lock_x);
	}
	cout << "recv_func exiting" << endl; // Debug to confirm exit
	pthread_exit(NULL);
}
