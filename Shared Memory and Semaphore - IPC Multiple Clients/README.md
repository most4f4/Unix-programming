# Semaphore with Shared Memory - UNIX IPC

This project demonstrates inter-process communication (IPC) using **POSIX named semaphores** and **shared memory** in a multi-client setup.

## Overview

Three client programs (`client1`, `client2`, and `client3`) exchange messages using shared memory for data storage and named semaphores for synchronization.

- **Shared Memory** is used to store the message and metadata.
- **Named Semaphores** ensure that only one client accesses the shared memory at a time to avoid race conditions.
  The memory is allocated as follows:

  ```bash
  struct Memory
  int packet_no;
  unsigned short srcClientNo;
  unsigned short destClientNo;
  char message[BUF_LEN];
  };
  ```

## File Structure

- `client1.cpp`, `client2.cpp`, `client3.cpp`: Programs that simulate clients exchanging messages.
- `client.h`: Header file defining the shared memory structure and constants.
- `start.sh`: Shell script to start all clients with slight delays for synchronization.
- `Makefile`: Contains build instructions for compiling the clients.

### Features

- Uses POSIX shared memory for data exchange.
- Synchronization achieved through named semaphores (`sem_open`, `sem_wait`, `sem_post`).
- Each client sends and receives messages in a synchronized loop.
- Graceful cleanup of resources (`sem_close`, `sem_unlink`, `shm_unlink`).

## Files

- `client1.cpp`, `client2.cpp`, `client3.cpp`: The three client programs.
- `common.h`: Shared header with constants, shared memory struct, and semaphore name.
- `start.sh`: Script to launch all clients with delays and synchronization.

## How to Run

1. **Compile the clients**:

   ```bash
   make
   ```

2. **Start the clients using** `start.sh`:

   ```bash
   ./start.sh
   ```

   This script launches the clients with 1-second intervals and waits for them to finish.

3. **Expected Output**:
   Each client should display messages it receives from another client, synchronized properly.

## Notes

- `semName` is the system-wide name for the named semaphore.
- `ShmPTR` is a pointer to a `Memory` struct mapped to shared memory.
- `sem_post()` and `sem_wait()` are used to release/acquire the semaphore.
- `sleep(1)` is used between loops for pacing and synchronization stability.
- The project uses signal handling for clean termination.

## Dependencies

- GCC Compiler
- POSIX-compliant system (Linux/WSL)
- `pthread` library

## Cleanup

The program detaches and removes shared memory and named semaphores after execution.

## Video Peresentation

[Youtube](https://youtu.be/gff8NTRYiSI)

## Licence

- This program is for educational purposes and is free to use and modify.
