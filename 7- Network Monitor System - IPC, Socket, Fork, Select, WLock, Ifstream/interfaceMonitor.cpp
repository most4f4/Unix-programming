#include <iostream>      // For cout, cerr
#include <fstream>       // For ifstream
#include <sys/socket.h>  // For socket functions
#include <sys/un.h>      // For UNIX domain socket structures
#include <unistd.h>      // For close, read, write, sleep
#include <cstring>       // For memset, strcmp
#include <sys/ioctl.h>   // For ioctl
#include <net/if.h>      // For network interface structures
#include <fcntl.h>       // For fcntl, file locking
#include <errno.h>       // For error handling
#include <signal.h>      // For signal handling (sigaction)

using namespace std;

#define SOCKET_PATH "/tmp/assignment2"
#define BUFFER_SIZE 256
#define LOCK_FILE "./stdout_lock"
bool hasSetLinkUpFailed = false; // Flag to track if Set Link Up has failed

bool isRunning = true;

// Handling SIGINT signals
void signalHandler(int signal) {
	if (signal == SIGINT) {
		isRunning = false;
	}
	else {
		cout << endl << "Undefined signal received" << endl;
	}
}

// print statis
void printStats(const char* interface) {

	// Build the base path for the interface 
	string base = "/sys/class/net/" + string(interface) + "/";
	// Variable to hold the values we read
	string value;
	// Open and read each file one at a time
	std::ifstream file;

	// Read operstate
	file.open(base + "operstate");
	if (file.is_open()) {
		getline(file, value);
		file.close();
	}
	else {
		value = "unknown";  // Default for state
	}
	cout << "Interface:" << interface << " state:" << value;

	// Helper function to read a stat and return "0" if it fails
	auto read_stat = [&](const string& stat_path) {
		file.open(base + stat_path);
		if (file.is_open()) {
			getline(file, value);
			file.close();
			return value;
		}
		file.close();  // Close even if open fails (safe practice)
		return string("0");  // Default to "0" if file can't be read
	};

	// Lock stdout to ensure atomic printing
	int lock_fd = open(LOCK_FILE, O_CREAT | O_RDWR, 0777);
	if (lock_fd < 0) {
		perror("Failed to open lock file");
		return;
	}

	struct flock fl;
	fl.l_type = F_WRLCK;  // Exclusive write lock
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;

	if (fcntl(lock_fd, F_SETLKW, &fl) < 0) {
		perror("Failed to acquire lock on stdout");
		close(lock_fd);
		return;
	}

	// Print other stats with proper paths
	cout << " up_count:" << read_stat("carrier_up_count")
		<< " down_count:" << read_stat("carrier_down_count")
		<< "\nrx_bytes:" << read_stat("statistics/rx_bytes")
		<< " rx_dropped:" << read_stat("statistics/rx_dropped")
		<< " rx_errors:" << read_stat("statistics/rx_errors")
		<< " rx_packets:" << read_stat("statistics/rx_packets")
		<< "\ntx_bytes:" << read_stat("statistics/tx_bytes")
		<< " tx_dropped:" << read_stat("statistics/tx_dropped")
		<< " tx_errors:" << read_stat("statistics/tx_errors")
		<< " tx_packets:" << read_stat("statistics/tx_packets")
		<< "\n" << endl;  // Double newline at the end


	// Unlock stdout
	fl.l_type = F_UNLCK;
	if (fcntl(lock_fd, F_SETLK, &fl) < 0) {
		perror("Failed to release lock on stdout");
	}
	close(lock_fd);
}

int main(int argc, const char* argv[]) {

	// check the number of arguments
	if (argc != 2) {
		perror("the interface name needs to be provided");
		exit(1);
	}
	const char* interface = argv[1];

	// Configure the signal handler for SIGINT
	struct sigaction sa;
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	// set up the signal handler for SIGINT
	sigaction(SIGINT, &sa, nullptr);

	// Create socket
	int client_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (client_socket_fd < 0) {
		perror("opening stream socket");
		exit(1);
	}

	// Set up socket address
	struct sockaddr_un client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sun_family = AF_UNIX;
	strcpy(client_addr.sun_path, SOCKET_PATH);

	// Connect to server
	if (connect(client_socket_fd, (struct sockaddr*)&client_addr, sizeof(struct sockaddr_un)) < 0) {
		perror("connecting stream socket failed");
		close(client_socket_fd);
		exit(1);
	}

	#ifdef DEBUG
		cout << "[ client " << argv[1] << ", process ID " << getpid() << " ] : connected to server " << endl;
	#endif

	write(client_socket_fd, "Ready", sizeof("Ready"));

	#ifdef DEBUG
		cout << "[ client " << argv[1] << ", process ID " << getpid() << " ] : Sent Ready to network monitor" << endl;
	#endif


	// Set the socket to non-blocking mode
	int flags = fcntl(client_socket_fd, F_GETFL, 0);
	if (flags == -1) {
		perror("fcntl F_GETFL error");
		close(client_socket_fd);
		exit(1);
	}
	if (fcntl(client_socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("fcntl F_SETTFL error");
		close(client_socket_fd);
		exit(1);
	}

	char buffer[BUFFER_SIZE];
	bool monitoring = false;

	while (isRunning) {

		// Clear buffer
		memset(buffer, 0, BUFFER_SIZE);

		int bytesRead = read(client_socket_fd, buffer, sizeof(buffer) - 1);

		if (bytesRead > 0) {

			buffer[bytesRead] = '\0';

			#ifdef DEBUG
				cout << "[ client " << interface << ", process ID " << getpid() << " ] : Received message: " << buffer << endl; 
			#endif

			// Handle potential concatenated messages
			char* ptr = buffer;
			while (*ptr) {
				if (strncmp(ptr, "Monitor", 7) == 0) {
					write(client_socket_fd, "Monitoring", sizeof("Monitoring"));
					monitoring = true;
					ptr += 7;
				} else if (strncmp(ptr, "Set Link Up", 11) == 0) {
					if (!hasSetLinkUpFailed) {
						int fd = socket(AF_INET, SOCK_DGRAM, 0);
						struct ifreq ifr;
						strncpy(ifr.ifr_name, interface, IFNAMSIZ);
						ifr.ifr_flags |= IFF_UP;
						if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
							perror("IOCTL failed");
							hasSetLinkUpFailed = true;
						}
						else {
							cout << "[ client " << interface << ", process ID " << getpid() << " ] : Successfully set interface up" << endl;
						}
						close(fd);
					}
					ptr += 11;
				}
				else if (strncmp(ptr, "Shut Down", 9) == 0) {
					write(client_socket_fd, "Done", sizeof("Done"));
					isRunning = false;
					ptr += 9;
				}
				else {
					cerr << "[ client " << argv[1] << ", process ID " << getpid() << " ] : Error! underfined message received from server!" << endl;
					ptr++;
				}
			}
		} else if (bytesRead == 0) {
			// Socket closed by the networkMonitor
			cout << "[Client " << interface << "]: Server closed connection, shutting down..." << endl;
			isRunning = false;
		}
		else if (bytesRead < 0) {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				// Actual error (not just "no data available")
				perror("read failed");
				isRunning = false;
			}
			// If errno is EAGAIN or EWOULDBLOCK, no data is available, so continue
		}

		// If monitoring, poll statistics every second
		if (monitoring && isRunning) {
			printStats(interface);

			// Check if the interface is down
			ifstream operstate("/sys/class/net/" + string(interface) + "/operstate");
			string state;
			if (operstate >> state) {
				if (state == "down") {
					write(client_socket_fd, "Link Down", strlen("Link Down"));
					cout << "[ client " << argv[1] << ", process ID " << getpid() << " ] : Reported down status to server!" << endl;
				}
			}
			operstate.close();
		}
	sleep(1);
	}

	if (!isRunning) {
		write(client_socket_fd, "Done", sizeof("Done"));
		cerr << "[ client " << argv[1] << ", process ID " << getpid() << " ] : Sent Done to server!" << endl;
	}

	close(client_socket_fd);
	return 0;
}
