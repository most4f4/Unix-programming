#include <iostream>
#include <signal.h>          // sigaction
#include <stdlib.h>          // exit, EXIT_FAILURE
#include <sys/socket.h>     // socket
#include <errno.h>           // err
#include <stdlib.h>          // sockaddr_in
#include <arpa/inet.h>       // htons()
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <fcntl.h>


#define IP_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 4096
#define PORT 8080

// Function prototype for the receive thread.
void* receive_func(void* arg);

bool is_running;
using namespace std;
pthread_mutex_t lock;
pthread_t t1;
struct sockaddr_in server_addr, client_addr;
const char* log_file = "logServer.log";

static void signalHandler(int sig) {
	switch (sig) {
	case SIGINT:
		is_running = false;
		break;
	}
}

int main() {

	char buf[BUFFER_SIZE];
	pthread_mutex_init(&lock, NULL);

	// Configure signal handling for SIGINT
	struct sigaction action;
	action.sa_handler = signalHandler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGINT, &action, nullptr);

	// Create master (listening) socket for UDP communications
	int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (server_socket < 0) {
		cerr << "creating stream socket" << strerror(errno) << endl;
		exit(1);
	}

	// Set the master socket to non-blocking mode
	// Retrieve the current socket flags
	int socket_flags = fcntl(server_socket, F_GETFL, 0);
	// Handle error if unable to get socket flags  
	if (socket_flags == -1) {
		cerr << "Error getting socket flags: " << strerror(errno) << endl;
		close(server_socket);
		exit(1);
	}
	// Set the socket to non-blocking mode by adding the O_NONBLOCK flag 
	if (fcntl(server_socket, F_SETFL, socket_flags | O_NONBLOCK) == -1) {
		// Handle error if unable to set non-blocking mode  
		cerr << "Error setting socket to non-blocking mode: " << strerror(errno) << endl;
		close(server_socket);
		exit(1);
	}

	// Set up the server address structure (IPv4, localhost, port number)
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	// inet_pton() converts the character string SOCKET_PATH into a networkaddress structure in the AF_INET address family
	// then copies the network address structure to &server_addr.sin_addr
	if (inet_pton(AF_INET, IP_ADDRESS, &server_addr.sin_addr) == 0) {
		cout << "Error in IP address Conversion" << strerror(errno) << endl;
		close(server_socket);
		exit(1);
	}
	// htons() convert IP port number to TCP/IP network byte order, which is with the most significant byte first
	server_addr.sin_port = htons(PORT);

	// Bind the master socket to the specified address and port
	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) < 0) {
		cerr << "binding stream socket failed" << strerror(errno) << endl;
		close(server_socket);
		exit(1);
	}

	// inet_ntoa() converts an (Ipv4) Internet network address into an ASCII string
	cout << "Server Listening on " << inet_ntoa(server_addr.sin_addr) << endl;

	// Create a thread to handle socket reads from this client
	if (pthread_create(&t1, NULL, receive_func, &server_socket) != 0) {
		cout << "Failed to create receive thread" << strerror(errno) << endl;
		close(server_socket);
		exit(1);
	}

	is_running = true;

	while (is_running) {
		int user_selection;
		cout << "1. Set the Log level" << endl;
		cout << "2. Dump the Log file" << endl;
		cout << "0. Shut Down" << endl;
		cout << "> ";
		cin >> user_selection;

		switch (user_selection) {
		case 0: {
			is_running = false;
			break;
		}
		case 1: {
			int log_level;
			cout << "Set the log level:" << endl;
			cout << "1. DEBUG" << endl;
			cout << "2. WARNING" << endl;
			cout << "3. ERROR" << endl;
			cout << "4. CRITICAL" << endl;
			cout << "> ";
			cin >> log_level;

			// Send the log level to logger
			memset(buf, 0, sizeof(buf));
			int len = sprintf(buf, "Set Log Level=%d", log_level) + 1;

			if (sendto(server_socket, buf, len, 0, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
				cerr << "Failed to send a message: " << strerror(errno) << endl;
				close(server_socket);
				exit(1);
			}
			break;
		}
		case 2: {
			// Open the log file as read-only and dump its contents to console
			pthread_mutex_lock(&lock); //  Lock before touching shared resources
			ifstream file(log_file, ios::in);
			if (file.is_open()) {
				string line;
				while (getline(file, line)) {
					cout << line << endl;
				}
				file.close();
			}
			else {
				cerr << "Unable to open file " << log_file << endl;
				close(server_socket);
				exit(1);
			}
			pthread_mutex_unlock(&lock); //  Unlock

			cout << "Press any key to continue";
			cin.get();

			break;
		}
		default: {
			cout << "Invalid Option selected! \n" << endl;
			break;
		}
		}

	}

	pthread_join(t1, NULL);
	pthread_mutex_destroy(&lock);

	return 0;
}

void* receive_func(void* arg) {
	// Extract the client file descriptor from the argument  
	int socket_fd = *(int*)arg;
	char buffer[BUFFER_SIZE];

	socklen_t client_addr_len = sizeof(client_addr);

	//open the server logfile for write only with permissions rw-rw-rw-
	int openFlags = O_WRONLY | O_CREAT | O_APPEND;
	mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	while (is_running) {

		memset(buffer, 0, sizeof(buffer));
		ssize_t num_bytes = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
		if (num_bytes == -1) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				// No data available in non-blocking mode, try again later
				continue;
			}
			else {
				// Other errors, log and close the socket
				cerr << "Read error: " << strerror(errno) << endl;
				break;
			}
		}
		else if (num_bytes == 0) {
			// Sleep for a sec if nothing is received
			sleep(1);
			continue;
			// we could also skip the num_bytes == 0 case entirely, since it’s unlikely in UDP.
		}
		else {
			pthread_mutex_lock(&lock);

			buffer[num_bytes] = '\0'; // Null terminate just in case

			// Write log entry
			int file_fd = open(log_file, openFlags, filePerms);
			if (file_fd < 0) {
				cerr << "Failed to open server.log" << strerror(errno) << endl;
				exit(1);
			}
			if (write(file_fd, buffer, num_bytes) < 0) {
				cerr << "Failed to write data to file: " << strerror(errno) << endl;
				exit(1);
			}
			close(file_fd);
			pthread_mutex_unlock(&lock);
		}
	}
	pthread_exit(NULL);
}


