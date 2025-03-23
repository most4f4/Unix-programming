#include <errno.h>       // Provides error numbers (errno) for system call failures
#include <iostream>      // Includes std::cout, std::cin, std::cerr for input/output operations
#include <string.h>      // Includes functions like strlen() and strerror()
#include <stdio.h>       // Includes printf()
#include <sys/ioctl.h>   // Provides ioctl() function for device-specific input/output operations
#include <net/if.h>      // Defines struct ifreq for network interface requests
#include <net/if_arp.h>  // Defines ARPHRD_ETHER for checking Ethernet interface
#include <arpa/inet.h>   // Includes inet_ntoa() for converting network addresses to string format
#include <unistd.h>      // Provides close() function for closing sockets

#define NAME_SIZE 16

using namespace std;

int main()
{
    int sockfd;
    int ret;
    int selection;
    struct ifreq ifr;
    char if_name[NAME_SIZE];
    unsigned char* mac=NULL;

    cout << "Enter the interface name: ";
    cin >> if_name;

    size_t if_name_len=strlen(if_name);
    if (if_name_len<sizeof(ifr.ifr_name)) {
        memcpy(ifr.ifr_name,if_name,if_name_len);
        ifr.ifr_name[if_name_len]=0;
    } else {
        cout << "Interface name is too long!" << endl;
	return -1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd<0) {
        cout << strerror(errno);
	return -1;
    }

    system("clear");
    do {
        cout << "Choose from the following:" << endl;
	cout << "1. Hardware address" << endl;
	cout << "2. IP address" << endl;
	cout << "3. Network mask" << endl;
	cout << "4. Broadcast address" << endl;
	cout << "0. Exit" << endl << endl;
	cin >> selection;
	switch(selection) {
        case 1:
            ret = ioctl(sockfd, SIOCGIFHWADDR, &ifr);
            if(ret<0) {
                cout << strerror(errno);
            } else if(ifr.ifr_hwaddr.sa_family!=ARPHRD_ETHER) {
                cout << "not an Ethernet interface" << endl;
            } else {
                mac=(unsigned char*)ifr.ifr_hwaddr.sa_data;
                printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	    }
            break;
        case 2:
            if (ioctl(sockfd, SIOCGIFADDR,&ifr ) == -1) {
                perror("ioctl");
                break;
            }
            else {
                struct sockaddr_in* ip = (struct sockaddr_in*)&ifr.ifr_addr;
                printf("IP Address of %s: %s\n", if_name, inet_ntoa(ip->sin_addr));
            }
            break;
        case 3:
            if (ioctl(sockfd, SIOCGIFNETMASK, &ifr) == -1) {
                perror("ioctl");
                break;
            }
            else {
                struct sockaddr_in* ip = (struct sockaddr_in*)&ifr.ifr_netmask;
                printf("Network Mask of %s: %s\n", if_name, inet_ntoa(ip->sin_addr));
            }

            break;
        case 4:           
            if (ioctl(sockfd, SIOCGIFBRDADDR, &ifr) == -1) {
                perror("ioctl");
                break;
            }
            else {
                struct sockaddr_in* ip = (struct sockaddr_in*)&ifr.ifr_broadaddr;
                printf("Broadcast Address of %s: %s\n", if_name, inet_ntoa(ip->sin_addr));
            }
            break;
        }
	if(selection!=0) {
            char key;
            cout << "Press any key to continue: ";
            cin >> key;
            system("clear");
        }
    } while (selection!=0);


    close(sockfd); 
    return 0;
}

