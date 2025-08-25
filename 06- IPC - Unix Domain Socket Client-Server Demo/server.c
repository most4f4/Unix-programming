#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SOCKET_PATH "/tmp/lab6"

int main(void)
{
	int server_socket, client_sock;
	struct sockaddr_un server_addr;
	char buffer[128];

	// Create socket
	server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_socket < 0) {
		perror("creating stream socket");
		exit(1);
	}

	// Set up socket address
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, SOCKET_PATH);

	// Bind the socket to the file path
	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un)) < 0) {
		perror("binding stream socket failed");
		close(server_socket);
		exit(1);
	}

	//  Listen for incoming connections
	if (listen(server_socket, 5) < 0) {
		perror("Server listen failed\n");
		close(server_socket);
		unlink(SOCKET_PATH);
		exit(1);
	}

	printf("Waiting for the client...\n");

	// Accept a connection
	client_sock = accept(server_socket, 0, 0);
	if (client_sock == -1) {
		perror("accept failed");
		close(server_socket);
		unlink(SOCKET_PATH);
		exit(1);
	}

	printf("client connected to the server\n");
	printf("server: accept()\n");

	// Request client's PID
	if (write(client_sock, "pid", 3) < 0)
		perror("Sending pid request failed\n");
	printf("The server requests the client's pid\n");

	// Receive client's pid response
	memset(buffer, 0, sizeof(buffer));
	if (read(client_sock, buffer, sizeof(buffer)) < 0)
		perror("reading stream message");
	printf("server: This client has pid %s\n", buffer);

	// Request client to sleep
	if (write(client_sock, "sleep", 5) < 0)
		perror("Sending sleep request failed\n");
	printf("The server requests the client's to sleep\n");

	// Receive client's sleep response
	memset(buffer, 0, sizeof(buffer));
	if (read(client_sock, buffer, sizeof(buffer)) < 0)
		perror("reading stream message");

	// Request client to quit
	if (write(client_sock, "quit", 4) < 0)
		perror("Sending quit request failed\n");
	printf("The server requests the client's to quit\n");

	// Cleanup
	close(client_sock);
	close(server_socket);
	unlink(SOCKET_PATH);

	return 0;
}