#include <iostream>		  // For cout and cin objects
#include <fcntl.h>        // For open() and file control options
#include <unistd.h>       // For close() and POSIX API
#include <linux/fb.h>     // For framebuffer structures and definitions
#include <sys/ioctl.h>    // For ioctl() system call
#include <stdio.h>        // For standard I/O functions
#include <errno.h>        // For error number definitions
#include <string.h>       // For strerror() to convert error numbers to



using namespace std; 

int main() {
	int selection = 1;
	int fd;

	while (selection != 0) {
		cout << "Please select one of the options:" << endl;
		cout << "1. Fixed Screen Info\n";
		cout << "2. Variable Screen Info\n";
		cout << "0. Exit\n\n";
		cout << "> ";
		cin >> selection;


		switch (selection)
		{
		case 1:
			// Open the framebuffer device in read-only, non-blocking mode
			fd = open("/dev/fb0", O_RDONLY, O_NONBLOCK);

			if (fd < 0) {
				cerr << "Error opening the device file : " << strerror(errno) << endl;
				return 1;
			}

			// Structure to hold fixed screen information
			struct fb_fix_screeninfo fix_info;

			if (ioctl(fd, FBIOGET_FSCREENINFO, &fix_info) < 0) {
				// If retrieval fails, print the error, close the file descriptor, and exit
				cerr << "Error retrieving fixed screen info: " << strerror(errno) << endl;
				return 1;
			}

			// Display the retrieved fixed screen information
			cout << "\nFixed Screen Info: \n";
			cout << "Visual: " << fix_info.visual << endl;
			cout << "Accelaration: " << fix_info.accel << endl;
			cout << "Capabilities: " << fix_info.capabilities << endl << endl;

			// Close the framebuffer device
			close(fd);
			break;
		case 2:
			// Open the framebuffer device in read-only, non-blocking mode
			fd = open("/dev/fb0", O_RDONLY, O_NONBLOCK);

			if (fd < 0) {
				cerr << "Error opening the device file : " << strerror(errno) << endl;
				return 1;
			}

			// Structure to hold variable screen information
			struct fb_var_screeninfo var_info;

			if (ioctl(fd, FBIOGET_VSCREENINFO, &var_info) < 0) {
				// If retrieval fails, print the error, close the file descriptor, and exit
				cerr << "Error retrieving variable screen info: " << strerror(errno) << endl;
				return 1;
			}

			// Display the retrieved fixed screen information
			cout << "\nVariable Screen Info: \n";
			cout << "Screen X Resolution: " << var_info.xres << endl;
			cout << "Screen Y Resolution: " << var_info.yres << endl;
			cout << "Bits per Pixel: " << var_info.bits_per_pixel << endl <<endl;

			// Close the framebuffer device
			close(fd);
			break;
		default:
			break;
		}
	}

	return 0;
}