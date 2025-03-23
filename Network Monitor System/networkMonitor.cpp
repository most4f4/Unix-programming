#include <cstring>        // For memset(), strcpy(), strcmp() (string manipulation)
#include <iostream>       // For cout, cerr, cin (standard input/output operations)
#include <sys/types.h>    // For pid_t (process ID type used in fork and waitpid)
#include <sys/socket.h>   // For socket(), bind(), listen(), accept(), write(), read() (socket programming)
#include <sys/un.h>       // For sockaddr_un (UNIX domain sockets)
#include <sys/wait.h>     // For waitpid(), WNOHANG (process management to handle child processes)
#include <unistd.h>       // For fork(), execlp(), unlink(), close(), write(), read() (POSIX API functions)
#include <string>         // For string 
#include <vector>         // For vector 

using namespace std;

#define SOCKET_PATH "/tmp/assignment2"
#define BUFFER_SIZE 256
#define MAX_NUMBER_CONNECTIONS 10

bool isRunning = true;
vector<int> monitor_sockets;
vector<pid_t> child_pids;

// Handling SIGINT signals
void signalHandler(int signal) {
	if (signal == SIGINT) {
		cout << "\nServer received CTRL-C Signal - shutting down" << endl;
		isRunning = false;
	}
	else {
		cout << endl << "Undefined signal received" << endl;
	}
}


// Shut down existing clients
void shutdownClients() {
	char shutdown_msg[] = "Shut Down";
	for (int client : monitor_sockets) {
		write(client, shutdown_msg, sizeof(shutdown_msg));
		close(client);
	}
	for (pid_t pid : child_pids) {
		if (pid > 0) {
			kill(pid, SIGINT);
			waitpid(pid, nullptr, 0);
		}
	}
	monitor_sockets.clear();
	unlink(SOCKET_PATH);
}

int main() {
	struct sockaddr_un server_addr;
	int server_socket, client_socket;
	char buffer[BUFFER_SIZE];

	// Get the number of Interfaces from the user
	int numInterfaces;
	cout << "Please enter the number of interfaces to monitor: ";
	cin >> numInterfaces;

	// Get the name of Interfaces from the user
	vector<string> interfaceNames(numInterfaces);
	child_pids.resize(numInterfaces);
	for (int i = 0; i < numInterfaces; i++) {
		cout << "Interface number " << i + 1 << ": ";
		cin >> interfaceNames[i];
	}

	// Configure and set up the signal handler for SIGINT
	struct sigaction sa;
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, nullptr);

	// Create socket
	server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_socket < 0) {
		perror("creating stream socket");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, SOCKET_PATH);

	// Remove any existing socket file having similar path 
	unlink(SOCKET_PATH);

	// Bind the socket to the file path
	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un)) < 0) {
		perror("binding stream socket failed");
		close(server_socket);
		exit(1);
	}

	// Listen for incoming connections
	if (listen(server_socket, MAX_NUMBER_CONNECTIONS) < 0) {
		cerr << "Server listen failed: " << strerror(errno) << endl;
		close(server_socket);
		unlink(SOCKET_PATH);
		exit(1);
	}
#ifdef DEBUG
	cout << "[ SERVER ] : Waiting for incoming connection..." << endl;
#endif
	for (int i = 0; i < numInterfaces; i++) {
		child_pids[i] = fork();
		if (child_pids[i] < 0) {
			cerr << "Forking failed: " << strerror(errno) << endl;
			exit(1);
		}
		else if (child_pids[i] == 0) {
			execlp("./interfaceMonitor", "./interfaceMonitor", interfaceNames[i].c_str(), nullptr);
			cerr << "Failed to execute interfaceMonitor for interface " << interfaceNames[i] << " --> " << strerror(errno) << endl;
			exit(1);
		}
	}

#ifdef DEBUG
	cout << "[ SERVER ] : Initiated interface monitors" << endl;
#endif

	//  Set of file descriptors to monitor for incoming data.
	fd_set readfds;
	// highest file descriptor value, needed for select()
	int max_socket = server_socket;

	for (int i = 0; i < numInterfaces; i++) {
		client_socket = accept(server_socket, nullptr, nullptr);
		if (client_socket < 0) {
			cerr << "Accepting connection failed: " << strerror(errno) << endl;
			shutdownClients();
			close(server_socket);
			exit(1);
		}
		monitor_sockets.push_back(client_socket);
		max_socket = max(max_socket, client_socket);
#ifdef DEBUG
		cout << "[ SERVER ] : Connection accepted via socket: " << client_socket << endl;
#endif
	}

	const char monitor[] = "Monitor";
	const char link_up[] = "Set Link Up";

	while (isRunning) {

		// clears(resets) the file descriptor set.
		FD_ZERO(&readfds);

		// Adds a socket (server_socket) to the readfds set.
		FD_SET(server_socket, &readfds);

		for (int sock : monitor_sockets) {
			FD_SET(sock, &readfds);
		}

		int result = select(max_socket + 1, &readfds, nullptr, nullptr, nullptr);
		if (result < 0 && errno != EINTR) {
			cerr << "select error: " << strerror(errno) << endl;
			break;
		}

		for (size_t i = 0; i < monitor_sockets.size(); i++) {

			// Checks if monitor_sockets[i] is ready for reading.
			if (FD_ISSET(monitor_sockets[i], &readfds)) {

				// Clear buffer
				memset(buffer, 0, BUFFER_SIZE);

				int bytesRead = read(monitor_sockets[i], buffer, BUFFER_SIZE);

				if (bytesRead > 0) {
					if (strcmp(buffer, "Ready") == 0) {
						write(monitor_sockets[i], monitor, strlen(monitor));
#ifdef DEBUG
						cout << "[ SERVER ] : Sent Monitoring command to socket " << monitor_sockets[i] << endl;
#endif
					}
					else if (strcmp(buffer, "Link Down") == 0) {
#ifdef DEBUG
						cout << "[ SERVER ] : received down status from client " << monitor_sockets[i] << endl;
#endif
						write(monitor_sockets[i], link_up, strlen(link_up));
					}
					else if (strcmp(buffer, "Done") == 0) {
						isRunning = false;
					}
					else {
						write(monitor_sockets[i], monitor, strlen(monitor));
					}
				}
				else if (bytesRead == 0) {
					FD_CLR(monitor_sockets[i], &readfds);
				}
				else {
					cerr << "read error" << endl;
					isRunning = false;
				}
			}
		}
	}

	shutdownClients();
	close(server_socket);

	return 0;
}
