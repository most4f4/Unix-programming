#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>  // For exit()
#include <unistd.h>  // For close() and write()
#include <string.h>	 // for strcmp() 
#include <stdbool.h> // for while(true)

#define SOCKET_PATH "/tmp/lab6"

int main(void) {

	int client_socket;
	struct sockaddr_un server_addr;
	char buffer[128];

	// Create socket
	client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (client_socket < 0) {
		perror("opening stream socket");
		exit(1);
	}

	// Set up socket address
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, SOCKET_PATH);

	// Connect to server
	if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un)) < 0) {
		perror("connecting stream socket failed");
		close(client_socket);
		exit(1);
	}

	// Communication loop
	while (true) {
		memset(buffer, 0, sizeof(buffer));

		// Read command from server
		if (read(client_socket, buffer, sizeof(buffer)) < 0) {
			perror("Reading stream message from server failed\n");
			break;
		}

		if (strcmp(buffer, "pid") == 0) {
			printf("A request for the client's pid has been received\n");
			snprintf(buffer, sizeof(buffer), "%d", getpid());
			if (write(client_socket, buffer, sizeof(buffer)) < 0) {
				perror("Sending response failed");
			}

		}
		else if (strcmp(buffer, "sleep") == 0) {
			printf("This client is going to sleep for 5 seconds\n");
			sleep(5);
			if (write(client_socket, "Done", 4) < 0) {
				perror("Sending response failed\n");
			}
		}
		else if (strcmp(buffer, "quit") == 0) {
			printf("This client is quitting\n");
			// exit from the while loop
			break;
		}
	}

	// Close client socket and unlink the socket file
	close(client_socket);

	return 0;
}


