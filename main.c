#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>

#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include "header.h"
#include "serial_func.h"
#include "curses_func.h"

//an important feature fot future implementation is to be able
//to open the interface with no ports active. In other words, I'd need
//to separate the interface itself from reading from ports as such.

WINDOW *console1_box;
WINDOW *console2_box;

WINDOW *console1;
WINDOW *console2;


int errfile;
int serial_port;

void exit_prep();
void exit_properly_SIGINT();
void exit_properly_SIGSEGV();

int main() {
	//error logging
	char *current_dir = getcwd(NULL, 0);
	char *errfile_path = (char *) calloc((strlen(current_dir) + 11), sizeof(char));
	sprintf(errfile_path, "%s/errorfile", current_dir);
	errfile = open(errfile_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	log_error("Error file opened successfully!\n");
	//write(errfile, "Error file opened successfully!\n", 33);
	if(errfile < 0) {
		fprintf(stderr, "lol fd:%d\n", errfile);
		fprintf(stderr, "error opening errorfile: %d, %s\n", errno, strerror(errno));
		//exit(errno);
	}
	free(current_dir);
	//free(errfile_path);

	//Configure serial communication:
	char *user_input = (char *) calloc(1024, sizeof(char));
	int char_count = 0;
	int incoming_byte = 0;
	int serial_port = open("/dev/ttyACM0", O_RDWR);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 10;

	if(serial_port < 0) {
		fprintf(stderr, "error: %d, %s\n", errno, strerror(errno));
		//exit(errno);
	}
	struct termios tty;
	if(tcgetattr(serial_port, &tty) != 0) {
		fprintf(stderr, "error: %d, %s\n", errno, strerror(errno));
	}

	configSerial(&tty, &serial_port);
	tcflush(serial_port, TCIOFLUSH);
	
	//Here begins the curses section of the configuration
	struct consoles ass = init_curses();
	console1 = ass.console1;
	console2 = ass.console2;
	console1_box = ass.console1_box;
	console2_box = ass.console2_box;

	if (has_colors() == FALSE) {
		endwin();
		printf("Your terminal does not support colors! Exiting.\n");
		exit(1);
	}

	int rv = 0; //for storing select return value
	int curr_curs_x, curr_curs_y; //for detecting backspaces

	fd_set set;
	FD_ZERO(&set);
	FD_SET(serial_port, &set);
	
	//some preliminary config
	signal(SIGINT, &exit_properly_SIGINT); //make sigint take curses into account
	signal(SIGSEGV, &exit_properly_SIGSEGV); //make sigint take curses into account


	//wmove(console1, LINES - 2, 1);
	
	struct div_disp boxes;
	boxes = create_div_disp(10);
	draw_div_boxes(boxes, console2);

	//re-implement whole next section!
	//plus corresponding func.c bits
	while(1) {
		//error log file
		//check if there is anything to be read:
		FD_ZERO(&set);
		FD_SET(serial_port, &set);
		rv = select(8, &set, NULL, NULL, &timeout);

		if(rv) {
			read(serial_port, &incoming_byte, 1);
			log_error("caught byte %d!\n", incoming_byte);
			print_div_byte(console2, &boxes, incoming_byte);
		}

		user_input[char_count] = wgetch(console1);
		if(user_input[char_count] == ERR) {
			continue;
		}
		if(user_input[char_count] == 0x7f) {
			user_input[char_count] = 0;
			user_input[char_count - 1] = 0;
			char_count -= 1;
			getyx(console1, curr_curs_y, curr_curs_x);
			mvwaddch(console1, curr_curs_y, curr_curs_x - 1, ' ');
			wmove(console1, curr_curs_y, curr_curs_x - 1);
			wrefresh(console1);
			continue;
		}
		//check if allowed character:
		if(!check_allowed_char(user_input[char_count])) {
				continue;
		}

		waddch(console1, user_input[char_count]);
		if(user_input[char_count] == '\n') {
			//process given line
			wattron(console1, COLOR_PAIR(2));
			wattr_on(console1, A_BOLD, NULL);
			user_input[char_count] = 0;
			interpret(user_input, console1, serial_port, &boxes);
			wattr_on(console1, A_BOLD, NULL);
			wattron(console1, COLOR_PAIR(1));

			char_count = 0;
			continue;
		}
		char_count++;
	}

	endwin();
	return 0;
}
void exit_prep() {
	delwin(console1);
	delwin(console2);
	delwin(console1_box);
	delwin(console2_box);
}
void exit_properly_SIGINT() 
{
	exit_prep();	
	endwin();
	fprintf(stderr, "SIGINT\n");
	fflush(stderr);
	exit(0);
}
void exit_properly_SIGSEGV() 
{
	exit_prep();	
	endwin();
	printf("SIGSEGV\n");
	fflush(stdout);
	exit(-2);
}
