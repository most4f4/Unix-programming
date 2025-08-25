#include <iostream>       // For cout, cerr
#include <sys/socket.h>   // For socket(), bind(), listen(), accept()
#include <netinet/in.h>   // For sockaddr_in
#include <unistd.h>       // For close(), read(), write(), sleep()
#include <pthread.h>      // For pthread_create(), pthread_join()
#include <cstring>        // For memset(), strerror()
#include <arpa/inet.h>    // For inet_pton()
#include <fcntl.h>        // For fcntl()
#include <queue>          // For std::queue
#include <signal.h>       // For sigaction()


using namespace std;

#define SOCKET_PATH "127.0.0.1"
#define BUFFER_SIZE 4096
#define MAX_NUMBER_CONNECTIONS 3

// Function prototype for the client receive thread.
void* receiveThread(void* arg);

bool is_running = true;
queue<string> message;
pthread_mutex_t lock_x = PTHREAD_MUTEX_INITIALIZER;

// Signal handler: sets is_running to false on SIGINT (Ctrl-C)
void signalHandler(int signal) {
	if (signal == SIGINT) {
		cout << "\nServer received CTRL-C Signal - shutting down" << endl;
		is_running = false;
	}
	else {
		cout << endl << "Undefined signal received" << endl;
	}
}


int main(int argc, const char* argv[])
{
	// Verify port number is provided as argument
	if (argc != 2) {
		perror("Port Number should be provided as argument.\n");
		exit(1);
	}

	// Configure signal handling for SIGINT
	struct sigaction action;
	action.sa_handler = signalHandler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGINT, &action, nullptr);

	// Create master (listening) socket
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		perror("creating stream socket");
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

	// Set up the server address structure (IPv4, localhost, port from argv)
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, SOCKET_PATH, &server_addr.sin_addr);
	server_addr.sin_port = htons(atoi(argv[1]));

	// Bind the master socket to the specified address and port
	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("binding stream socket failed");
		close(server_socket);
		exit(1);
	}

	//  Listen for incoming connections
	if (listen(server_socket, MAX_NUMBER_CONNECTIONS) < 0) {
		perror("Server listen failed\n");
		close(server_socket);
		exit(1);
	}

	printf("Waiting for incoming connection...\n");

	int active_clients = 0;
	int client_sockets[MAX_NUMBER_CONNECTIONS];
	pthread_t client_threads[MAX_NUMBER_CONNECTIONS];

	while (is_running) {
		// Accept new client if limit not reached
		if (active_clients < MAX_NUMBER_CONNECTIONS) {
			client_sockets[active_clients] = accept(server_socket, nullptr, nullptr);
			if (client_sockets[active_clients] < 0) {
				// If no pending connection to accept, continue looping
				if (errno == EWOULDBLOCK || errno == EAGAIN) {
					continue;
				}
				else {
					cerr << "Accepting connection failed: " << strerror(errno) << endl;
					close(server_socket);
					exit(1);
				}
			}
			// Create a thread to handle socket reads from this client
			if (pthread_create(&client_threads[active_clients], NULL, receiveThread, &client_sockets[active_clients]) != 0) {
				cout << "Failed to create receive thread" << strerror(errno) << endl;
				close(server_socket);
				close(client_sockets[active_clients]);
				exit(1);
			}
			active_clients++;
		}
		// Process and print messages from the message queue
		while (!message.empty()) {
			pthread_mutex_lock(&lock_x);
			cout << message.front() << endl;
			message.pop();
			pthread_mutex_unlock(&lock_x);
		}
		
		sleep(1);
	}

	// Gracefully shut down: signal clients to quit, join threads, and close sockets
	for (int i = 0; i < active_clients; ++i) {
		write(client_sockets[i], "Quit", strlen("Quit"));
		pthread_join(client_threads[i], NULL);
		close(client_sockets[i]);
	}

	cout << endl << "Server is shutting down..." << endl;
	close(server_socket);

	return 0;
}

void* receiveThread(void* arg) {
	// Extract the client file descriptor from the argument  
	int client_fd = *(int*)arg;

	// Set a read timeout of 5 seconds on the client socket
	// Create a timeval structure to specify the timeout duration
	struct timeval tv;
	tv.tv_sec = 5;  // Timeout after 5 seconds  
	tv.tv_usec = 0; // No additional microseconds  

	// Apply the receive timeout option to the client socket  
	// SOL_SOCKET indicates that the option is at the socket API level
	// SO_RCVTIMEO sets the timeout value for receiving data
	setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	char buffer[BUFFER_SIZE];

	while (is_running) {

		// Clear buffer
		memset(buffer, 0, BUFFER_SIZE);
		int num_bytes = read(client_fd, buffer, BUFFER_SIZE);
		
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
		} else if (num_bytes == 0) {
			// Client closed the connection
			break;
		}
		else {
			// Push received message into the shared queue (protected by a mutex)
			pthread_mutex_lock(&lock_x);
			message.push(string(buffer));
			pthread_mutex_unlock(&lock_x);
		}

	}
	pthread_exit(NULL);
}