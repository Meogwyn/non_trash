#include <stdio.h>
#include <stdlib.h>
#include "header.h"
#include <unistd.h>
#include <string.h>

#include <sys/select.h>

#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions

int main () {
	int serial_port = open("/dev/ttyACM0", O_RDWR);
	char *incoming_bytes = (char *) calloc(200, sizeof(char));
	char outgoing_byte;
	char *userStr = (char *) calloc(200, sizeof(char));
	int select_return_value = -1;
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	
	/*
	//fd stuff for select()
	fd_set set;
	FD_ZERO(&set);
	FD_SET(serial_port, &set);
	*/



	if(serial_port < 0) {
		fprintf(stderr, "lel: %i, %s\n", errno, strerror(errno));
	}
	struct termios tty;
	if(tcgetattr(serial_port, &tty) != 0) {
		fprintf(stderr, "lol1:%i, %s", errno, strerror(errno));
	}

	configSerial(&tty, &serial_port);
	
	while(1) {
		printf("input a byte to send to the arduino:\n");
		scanf("%s", userStr);	
		outgoing_byte = (unsigned int) strtoul(userStr, NULL, 2);
		
		//read-back:
		for(int i = 0; i < sizeof(char)*8; i++) {
			if(outgoing_byte & (1 << (7 - i)))
				printf("1");
			else
				printf("0");

		}
		printf("\nREADBACK DONE\n");

		//now we send it.
		if(write(serial_port, &outgoing_byte, 1) < 0) {
			printf("lol2:%d, %s\n", errno, strerror(errno));
		}
		printf("Reached reading stage\n");
		while(read(serial_port, incoming_bytes, 1)) {/*
			select_return_value = select(serial_port, &set, NULL, NULL, &timeout);
			if(select_return_value < 0) {
				printf("lol3: %d, %s\n", errno, strerror(errno));
				break;
			}
			if(select_return_value == 0) {
				printf("didn't receive anything!\n");
				break;
			}*/
			printf("incoming byte:%d\n", (int)incoming_bytes[0]);
			fflush(stdout);
		}
	}

	return 0;
}
