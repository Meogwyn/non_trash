#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include "serial_func.h"

void configSerial(struct termios *tty, int *serial_port) { //the int is for serial port fd
	tty->c_cflag &= ~PARENB; //clear the PARENB bit for no parity
	tty->c_cflag &= ~CSTOPB; //use only 1 stop bit, as per arduino default
	tty->c_cflag &= ~CSIZE; //clear all size bits
	tty->c_cflag |= CS8; //8 data bits
	tty->c_cflag &= ~CRTSCTS; //disable RTS/CTS flow control pins
	tty->c_cflag |= CREAD | CLOCAL;

	tty->c_lflag &= ~ICANON; //disable canonical mode
	tty->c_lflag &= ~ECHO; // Disable echo
	tty->c_lflag &= ~ECHOE; // Disable erasure
	tty->c_lflag &= ~ECHONL; // Disable new-line echo
	tty->c_lflag &= ~ISIG; //disable weird characters

	tty->c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	tty->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
	tty->c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty->c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed


	tty->c_cc[VTIME] = 0;
	tty->c_cc[VMIN] = 0;

	cfsetispeed(tty, B9600);
	cfsetospeed(tty, B9600);

	// Save tty settings, also checking for error
	if (tcsetattr(*serial_port, TCSANOW, tty) != 0) {
	    printf("error %i from tcsetattr: %s\n", errno, strerror(errno));
	}
}
