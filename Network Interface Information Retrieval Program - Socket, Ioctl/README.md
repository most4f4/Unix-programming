# Network Interface Information Retrieval Program

This program retrieves and displays network interface information on a Linux system. It supports retrieving hardware (MAC) address, IP address, network mask, and broadcast address for a given network interface.

## Prerequisites

Before running the program, ensure the following:

- A Linux-based operating system
- A network interface (e.g., eth0, ens33, etc.)
- Development tools (C++ compiler, make)

## Compilation

To compile the program, run the following command:

```bash
make
```

This will produce an executable file named `net_info`.

## Usage

1. Run the program with:

   ```bash
   ./net_info
   ```

2. You will be prompted to enter the network interface name (e.g., `ens33`, `eth0`).
3. You can then choose from the following options:
   - **1**: Retrieve the MAC (hardware) address
   - **2**: Retrieve the IP address
   - **3**: Retrieve the network mask
   - **4**: Retrieve the broadcast address
   - **0**: Exit the program

### Example Interaction:

```text
Enter the interface name: ens33
Choose from the following:
1. Hardware address
2. IP address
3. Network mask
4. Broadcast address
0. Exit

1
MAC Address: 00:1A:2B:3C:4D:5E

Press any key to continue:
```

## Code Explanation

- **Main Logic**:
  The program uses `ioctl()` system calls to retrieve information about the given network interface, including hardware address, IP address, network mask, and broadcast address.
- **Network Interface**:
  The program works with any standard network interface like Ethernet or Wi-Fi.

- **Error Handling**:
  Errors during system calls are handled by checking the return values and displaying appropriate error messages.

## License

This project is for educational purposes.

## Video Peresentation

[Youtube](https://youtu.be/SJsMlIGMlKc)

## Acknowledgements

This code was developed as part of a system-level programming exercise.
