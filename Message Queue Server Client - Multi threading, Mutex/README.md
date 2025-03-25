# Server-Client Message Communication System

## Overview

This system allows a server to communicate with three clients. Clients send messages to the server, which forwards them to the appropriate destination based on the message type. The system uses message queues for communication and synchronization, ensuring messages are distributed efficiently between clients.

## Project Structure

- `server.cpp`: Implements the server that handles messages between clients.
- `client1.cpp`: Implements Client 1 that sends and receives messages.
- `client2.cpp`: Implements Client 2.
- `client3.cpp`: Implements Client 3.
- `Makefile`: Contains build instructions for compiling the server and clients.
- `startClient.sh`: Starts all three clients.
- `stop.sh`: Stops all running processes.

## Message Structure

The message structure is defined as follows:

```c
const int BUF_LEN = 64;

// Structure for message queue
typedef struct mesg_buffer {
    long source;  // Source client ID
    long dest;    // Destination client ID
    char buf[BUF_LEN];  // Message content
} MesgBuffer;

// Structure for message type
typedef struct mymsg {
    long mtype;  // Message type (used for filtering messages by client)
    MesgBuffer msgBuf;  // The message content
} Message;
```

### Message Types

- **mtype 4**: This is the message type the server will receive.
- **mtype 1**: Server sends messages to client 1.
- **mtype 2**: Server sends messages to client 2.
- **mtype 3**: Server sends messages to client 3.

## Message Queue Communication

- The **server** receives messages from clients and forwards them based on message type.
- Clients alternate sending messages to each other via the server.
- When shutting down, the server sends a `"Quit"` message to notify clients.

### Key

The communication key between the server and clients is:

```c
key = ftok("serverclient", 65);
```

### Server Requirements

- The server should have a message queue to store messages from all clients.
- The server uses a mutex to protect the message queue for thread safety.
- The server should have one receive thread that listens for incoming messages from clients.

### Client Requirements

- Each client sends its messages to the server for distribution.
- Clients filter messages by their respective message types.

### Shutdown Process

- The server performs a controlled shutdown on **Ctrl+C**.
- It ensures the message queue is cleared, and all threads are properly joined before exiting.

## Build and Run Instructions

### 1. Compile the Server and Clients

```sh
make
```

### 2. Start the Server

```sh
./server
```

### 3. Start the Clients

```sh
./startClient.sh
```

### 4. Stop the Processes

```sh
./stop.sh
```

### Isolating one client

For testing, you may isolate each client to observe message delivery and ensure proper functionality of the server-client communication.

```sh
./startClient.sh | grep 'client 3'
```

## Dependencies

- `g++` compiler
- POSIX threading (`pthread`)
- System V Message Queues.

## Video Peresentation

[Youtube](https://youtu.be/ZGt9NCoK3uE)

## Licence

- This program is for educational purposes and is free to use and modify.

## Acknowledgments

- Developed by: Mostafa Hasanalipourshahrabadi
- Tested on a Linux VM.
