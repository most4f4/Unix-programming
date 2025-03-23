# Network Monitor Project

## Overview

This project implements a **Network Monitor** system for monitoring network interfaces on a Linux system. It consists of two C++ programs:

- **`NetworkMonitor.cpp`**: A server that coordinates monitoring by spawning and managing multiple interface monitor processes.
- **`interfaceMonitor.cpp`**: A client process (one per network interface) that polls and displays interface statistics, manages link states, and communicates with the server.

The system uses **UNIX domain sockets** for inter-process communication, polls network statistics every second from `/sys/class/net/<interface-name>/`, and ensures a graceful shutdown via SIGINT (Ctrl-C). It fulfills the assignment requirements for monitoring interface performance, handling link state changes, and providing synchronized output.

## Features

- Monitors any user-specified set of network interfaces (e.g., `ens33`, `lo`).
- Displays statistics every second in the required format:
  
  ```text
  Interface:<interface-name> state:<state> up_count:<up-count> down_count:<down-count>
  rx_bytes:<rx-bytes> rx_dropped:<rx-dropped> rx_errors:<rx-errors> rx_packets:<rx-packets>
  tx_bytes:<tx-bytes> tx_dropped:<tx-dropped> tx_errors:<tx-errors> tx_packets:<tx-packets>
- Automatically brings an interface back up if it goes down using `ioctl`.
- Handles shutdown gracefully with Ctrl-C, cleaning up sockets and child processes.
- Supports debugging output with a `DEBUG` flag.
- Uses file locking to prevent interleaved output from multiple interface monitors.


## Prerequisites

- Operating System: Linux (tested on a virtual machine with interfaces like ens33 and lo).
- Compiler: g++ (GCC) for compiling C++ code.
- Libraries: Standard C++ and POSIX libraries (e.g., <sys/socket.h>, <unistd.h>).
- Permissions: Root privileges may be required to modify interface states with ioctl (e.g., sudo).

## Installation

## Clone the Repository:
```bash
git clone https://github.com/<your-username>/<your-repo-name>.git
cd <Network_Monitor_System>
```

## Compile the Code:
Use the provided Makefile to build the executables:
```bash
make
```
This creates `NetworkMonitor` and `interfaceMonitor` in the current directory.

## Compile with Debugging (Optional):
To enable debug output:
```bash
make debug
```

## Clean Up (Optional):
Remove compiled files:
```bash
make clean
```

# Usage

## Run the Network Monitor:
```bash
./NetworkMonitor
```
You’ll be prompted to enter the number of interfaces to monitor.
Then, enter each interface name (e.g., ens33, lo).

### Example Interaction:
```text
Please enter the number of interfaces to monitor: 2
Interface number 1: ens33
Interface number 2: lo
```
The program spawns two interfaceMonitor processes and starts monitoring.

### Output:
Each interfaceMonitor prints stats every second, e.g.:
```text
Interface:ens33 state:up up_count:1 down_count:0
rx_bytes:54321 rx_dropped:0 rx_errors:0 rx_packets:500
tx_bytes:12345 tx_dropped:0 tx_errors:0 tx_packets:200

Interface:lo state:up up_count:1 down_count:0
rx_bytes:12345 rx_dropped:0 rx_errors:0 rx_packets:100
tx_bytes:12345 tx_dropped:0 tx_errors:0 tx_packets:100
```

### Test Link Down: 
Simulate an interface going down:
```bash
sudo ip link set ens33 down
```

The interfaceMonitor detects this, sends "Link Down" to NetworkMonitor, which responds with "Set Link Up", and the interface is brought back up (if ioctl succeeds).

### Shutdown: 
Press Ctrl-C in the NetworkMonitor terminal:
```text
Server received CTRL-C Signal - shutting down
```

All interfaceMonitor processes shut down cleanly, and resources are freed.

## Project Structure

- **NetworkMonitor.cpp**: 
  - Server module that manages interface monitors.
  - Uses UNIX domain sockets at `/tmp/assignment2`.
  - Forks child processes and handles communication.
  
- **interfaceMonitor.cpp**: 
  - Client module that monitors a single interface.
  - Polls stats from `/sys/class/net/<interface-name>/`.
  - Implements link state management with `ioctl`.
  
- **Makefile**:
  - Builds the project with `make`.
  - Supports `make debug` for debugging output and `make clean` to remove executables.

## How It Works

### Startup:
- NetworkMonitor prompts for interfaces, forks interfaceMonitor processes, and sets up a UNIX socket server.
- Each interfaceMonitor connects to the socket and sends "Ready".

### Monitoring:
- NetworkMonitor sends "Monitor" to each client, which starts polling stats every second.
- If an interface goes down, the client sends "Link Down", and the server responds with "Set Link Up".

### Communication:
- Uses synchronous messaging over UNIX sockets (`/tmp/assignment2`).
- `select` in NetworkMonitor manages multiple client connections.

### Shutdown:
- Ctrl-C triggers SIGINT, prompting NetworkMonitor to send "Shut Down" and terminate child processes.
- Each interfaceMonitor sends "Done" and exits.

## Debugging

Compile with `make debug` to enable debug output:
```text
[ SERVER ] : Waiting for incoming connection...
[ SERVER ] : Connection accepted via socket: 4
[ client ens33, process ID 12345 ] : Sent Ready to network monitor
```

Debug statements are wrapped in `#ifdef DEBUG` blocks.

## Limitations

- Requires root privileges for `ioctl` to modify interface states.
- Assumes interface names are valid and exist on the system.
- No web interface.
- Error handling could be more robust.

## Future Improvements

- Add a web interface for stats display (bonus mark opportunity).
- Enhance error recovery for socket failures.
- Support dynamic addition/removal of interfaces without restarting.

## License

This project is for educational purposes. No formal license is applied.

## Acknowledgments

- Developed by: Mostafa Hasanalipourshahrabadi
- Tested on a Linux VM with interfaces ens33 and lo.
