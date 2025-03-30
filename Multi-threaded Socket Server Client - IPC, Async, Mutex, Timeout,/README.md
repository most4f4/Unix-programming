# Multi-Threaded Socket Server Client Communication System

## Overview

This is a simple multi-threaded socket server in C++ that listens for incoming client connections, receives messages asynchronously, and processes them using a shared queue with mutex protection.

## Features

- **Asynchronous Communication:** Each client connection runs in its own thread for non-blocking message handling.
- **Multi-threading with pthreads:** Uses `pthread_create()` for concurrent client handling.
- **Non-blocking Server Socket:** The server is set to non-blocking mode to avoid unnecessary blocking on `accept()`.
- **SIGINT Handling:** Gracefully shuts down when `CTRL+C` is pressed.
- **Thread-Safe Message Queue:** Uses `pthread_mutex_t` to avoid race conditions.

## How It Works

1. The server binds to `127.0.0.1` (localhost) on a specified port.
2. It listens for incoming client connections.
3. When a client connects, a new thread is created for receiving messages.
4. Messages are added to a queue, protected by a mutex.
5. The main thread prints messages from the queue.
6. On shutdown (`CTRL+C`), all client connections are closed.

## Build and Run Instructions

### 1. Compile the Server and Clients

```sh
make
```

### 2. Start the Server

```sh
./server <PORT_NUMBER>
```

Example:

```sh
./server 1153
```

### 3. Start the Clients in another terminal

```sh
./startClient.sh <PORT_NUMBER>
```

## Dependencies

- `g++` compiler
- POSIX threading (`pthread`)
- System V Message Queues.

## Video Peresentation

[Youtube](https://youtu.be/Bpov4v-EY7k)

## Licence

- This program is for educational purposes and is free to use and modify.

## Acknowledgments

- Developed by: Mostafa Hasanalipourshahrabadi
- Tested on a Linux VM.
