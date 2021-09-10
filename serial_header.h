#include <termios.h>
#include <fcntl.h>

void configSerial(struct termios *tty, int *serial_port);
