#include <iostream> 
#include <sys/socket.h>   // For socket()
#include <errno.h>        // For errno
#include <stdlib.h>       // For exit()
#include <cstring>        // For memset()
#include <netinet/in.h>   // For struct sockaddr_in 
#include <arpa/inet.h>    // For htons()
#include <cstring>	      // For memset()
#include <ctime>	      // For memset()
#include <string.h>       // For strncmp()
#include <fcntl.h>        // For fcntl()
#include <unistd.h>       // For close()
#include "Logger.h"

#define IP_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 4096
#define PORT 8080

// Function prototype for the receive thread.
void* receive_func(void* arg);

bool is_running = true;
char buf[BUFFER_SIZE];
struct sockaddr_in addr;
pthread_mutex_t lock;
pthread_t t1;
LOG_LEVEL log_filter = DEBUG; // default
int socket_fd, len;


using namespace std;

int InitializeLog() {
	// Create client socket for UDP communications with server
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0) {
		cerr << "creating stream socket" << strerror(errno) << endl;
		exit(1);
	}
	// Set the socket to non-blocking mode
	// Retrieve the current socket flags
	int socket_flags = fcntl(socket_fd, F_GETFL, 0);
	if (socket_flags == -1) {
		cerr << "Error getting socket flags: " << strerror(errno) << endl;
		close(socket_fd);
		exit(1);
	}

	// Set the socket to non-blocking mode by adding the O_NONBLOCK flag 
	if (fcntl(socket_fd, F_SETFL, socket_flags | O_NONBLOCK) == -1) {
		// Handle error if unable to set non-blocking mode  
		cerr << "Error setting socket to non-blocking mode: " << strerror(errno) << endl;
		close(socket_fd);
		exit(1);
	}

	// Set up the server address structure (IPv4, localhost, port number)
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	// inet_pton() converts the character string SOCKET_PATH into a networkaddress structure in the AF_INET address family
	// then copies the network address structure to &server_addr.sin_addr
	if (inet_pton(AF_INET, IP_ADDRESS, &addr.sin_addr) == 0) {
		cout << "Error in IP ADRESS Conversion" << strerror(errno) << endl;
		close(socket_fd);
		exit(1);
	}
	// htons() convert IP port number to TCP/IP network byte order, which is with the most significant byte first
	addr.sin_port = htons(PORT);


	cout << "Communication with Server on " << inet_ntoa(addr.sin_addr) << endl;

	const char message[] = "Logger can now communicate to the server\n";
	sendto(socket_fd, message, sizeof(message), 0, (struct sockaddr*)&addr, sizeof(addr));

	pthread_mutex_init(&lock, NULL);

	if (pthread_create(&t1, NULL, receive_func, &socket_fd) != 0) {
		cout << "Failed to create receive thread" << strerror(errno) << endl;
		close(socket_fd);
		exit(1);
	}

	return socket_fd;
}

void SetLogLevel(LOG_LEVEL level) {
	pthread_mutex_lock(&lock);
	log_filter = level;
	cout << "Log filter set to: " << log_filter << endl;
	pthread_mutex_unlock(&lock);
}

void Log(LOG_LEVEL level, const char* file, const char* func, int line, const char* message) {
	
	if (level < log_filter) {
		cout << "[LOGGER] SKIPPED: " << level << endl;
		return;
	}
	cout << "[LOGGER] SENT: " << level << endl;

	pthread_mutex_lock(&lock);  //  Lock before touching shared resources
	if (level >= log_filter) {

		time_t now = time(0);   // Get current time
		char* dt = ctime(&now); // Convert to string format
		memset(buf, 0, sizeof(buf));
		char levelStr[][16] = { "DEBUG", "WARNING", "ERROR", "CRITICAL" };

		len = sprintf(buf, "%s %s %s:%s:%d %s\n", dt, levelStr[level], file, func, line, message) + 1;
		buf[len - 1] = '\0';

		// Send the log to the server
		if (sendto(socket_fd, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			cerr << "Failed to send a message to server" << strerror(errno) << endl;
			close(socket_fd);
			exit(1);
		}

		memset(buf, 0, sizeof(buf));

	}
	pthread_mutex_unlock(&lock);  //  Unlock after sending

}

void ExitLog() {
	cout << "Logger is shutting down" << endl;
	is_running = false;
	pthread_join(t1, NULL);
	pthread_mutex_destroy(&lock);
	close(socket_fd);
}

void* receive_func(void* arg) {
	int fd = *(int*)arg;
	socklen_t addr_len = sizeof(addr);

	while (is_running) {
		pthread_mutex_lock(&lock);
		ssize_t num_bytes = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len);
		pthread_mutex_unlock(&lock);

		// Sleep a sec if nothing is received
		if (num_bytes < 0) {
			sleep(1);

		}
		else {
			buf[num_bytes] = '\0'; // Ensure null termination
			cout << buf << endl; 

			if (strncmp(buf, "Set Log Level=", strlen("Set Log Level=")) == 0) {

				int logLevel;
				sscanf(buf, "Set Log Level=%d", &logLevel);
				LOG_LEVEL severity;
				switch (logLevel) {
				case 1:
					severity = DEBUG;
					break;
				case 2:
					severity = WARNING;
					break;
				case 3:
					severity = ERROR;
					break;
				case 4:
					severity = CRITICAL;
					break;
				default:
					cerr << "Invalid log level: " << logLevel << endl;
				}
				SetLogLevel(severity);
			}

		}
	}
	pthread_exit(NULL);
}