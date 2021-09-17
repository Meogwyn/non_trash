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

#define WIDTH COLS/2
#define HEIGHT LINES




WINDOW *console1_box;
WINDOW *console2_box;

WINDOW *console1;
WINDOW *console2;

//void testy();

int errfile;
int serial_port;

int main() {
	//error logging
	char *current_dir = getcwd(NULL, 0);
	char *errfile_path = (char *) malloc((strlen(current_dir) + 10) * sizeof(char));
	sprintf(errfile_path, "%s/errorfile");
	errfile = open(errfile_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if(errfile < 0) {
		fprintf(stderr, "lol ");
		fprintf(stderr, "error opening errorfile: %d, %s\n", errno, strerror(errno));
		//exit(errno);
	}
	free(current_dir);
	free(errfile_path);
	//some preliminary config
	signal(SIGINT, &exit_properly); //make sigint take curses into account
	signal(SIGSEGV, &exit_properly); //make sigint take curses into account

	//Configure serial communication:
	int incoming_byte = 0;
	int serial_port = open("/dev/ttyACM1", O_RDWR);
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
	console1 = ass.console1;
	console2_box = ass.console2_box;

	if (has_colors() == FALSE) {
		endwin();
		printf("Your terminal does not support colors! Exiting.\n");
		exit(1);
	}

	int rv = 0; //for storing select return value
	int prev_curs_x, prev_curs_y; //for detecting backspaces
	int curr_curs_x, curr_curs_y; //for detecting backspaces
	int to_be_removed; //for storing number of chars to be removed after backspace

	fd_set set;
	FD_ZERO(&set);
	FD_SET(serial_port, &set);


	wmove(console1, LINES - 2, 1);
	//testy();
	
	struct div_disp boxes;
	boxes = create_div_disp(10);
	//draw_div_boxes(&boxes);

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
			
			/*
			for(int i = 0; i < 8; i++) {
				if(incoming_byte & (1 << 7 - i)) {
					waddch(console2, '1');
				}
				else {
					waddch(console2, '0');
				}
			}*/
			//getyx(console2, prev_curs_y, prev_curs_x);
			//print_byte(console2, incoming_byte, prev_curs_y, prev_curs_x);
			//wprintw(console2, ", %u, %c", incoming_byte, incoming_byte);
			//waddch(console2, '\n');
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
void exit_properly() {
	delwin(console1);
	delwin(console2);
	delwin(console1_box);
	delwin(console2_box);
	endwin();
	printf("segfault\n");
	exit(0);
}
/*
void testy() {
	struct div_disp boxes;
	boxes = div_init(10);
	int number = 0;

	for(int i = 0; i < boxes.div_num_y && number < boxes.div; i++) {

		for(int k = 0; k < boxes.div_num_x && number < boxes.div; k++) {
			mvwaddch(console2, boxes.box_coords_yx[i][k][0], boxes.box_coords_yx[i][k][1], 'a');
			number++;
		}
	}
	free_stuff(&boxes);
	wgetch(console2);
	exit_properly();
}*/
