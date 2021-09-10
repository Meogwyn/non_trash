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
#include "serial_header.h"

#define WIDTH COLS/2
#define HEIGHT LINES




WINDOW *console1_box;
WINDOW *console2_box;

WINDOW *console1;
WINDOW *console2;

//void testy();

struct div_disp boxes;
int errfile;

int main() {
	//error logging
	errfile = open("/home/danus/devel/arduino/interface/non_trash/errorfile", O_WRONLY);
	if(errfile < 0) {
		fprintf(stderr, "lol ");
		fprintf(stderr, "error: %d, %s\n", errno, strerror(errno));
		exit(errno);
	}
	
	//some preliminary config
	boxes.div = 1; //default div value
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
		exit(errno);
	}
	struct termios tty;
	if(tcgetattr(serial_port, &tty) != 0) {
		fprintf(stderr, "error: %d, %s\n", errno, strerror(errno));
	}

	configSerial(&tty, &serial_port);
	tcflush(serial_port, TCIOFLUSH);
	
	//Here begins the curses section of the configuration
	init_curses();
	if (has_colors() == FALSE) {
		endwin();
		printf("Your terminal does not support colors! Exiting.\n");
		exit(1);
	}

	int startx = 0;
	int starty = 0;
	char *user_input = (char *) calloc(1024, sizeof(char));
	int char_count = 0;

	//Configure windows
	

	console1_box = create_window_box(HEIGHT, WIDTH, starty, startx);	
	
	startx = COLS/2;

	console2_box = create_window_box(HEIGHT, WIDTH, starty, startx);
	
	startx = 1;
	starty = 1;

	console1 = create_window_nobox(HEIGHT - 2, WIDTH - 2, starty, startx);
	scrollok(console1, TRUE);
	idlok(console1, TRUE);
	wsetscrreg(console1, 1, HEIGHT - 2);
	nodelay(console1, TRUE);
	curs_set(0);

	startx += COLS/2;

	
	console2 = create_window_nobox(HEIGHT - 2, WIDTH - 2, starty, startx);
	//scrollok(console2, TRUE);
	//idlok(console2, TRUE);
	//wsetscrreg(console2, 1, HEIGHT - 2);
	

	//Deal with color:
	start_color();
	
	init_pair(1, COLOR_YELLOW, COLOR_BLACK); //left terminal user text
	init_pair(2, COLOR_RED, COLOR_BLACK); //left terminal response text
	init_pair(3, COLOR_GREEN, COLOR_BLACK); //right terminal readback text
	init_pair(4, COLOR_CYAN, COLOR_BLACK); //right terminal readback text

	wattron(console1, COLOR_PAIR(1) | A_BOLD);
	//wattron(console2, COLOR_PAIR(3) | A_BOLD);
	wprintw(console1, "Hello, World!\n");
	wprintw(console2, "Hello, World!\n");
	wrefresh(console1);
	wrefresh(console2);

	int rv = 0; //for storing select return value
	int prev_curs_x, prev_curs_y; //for detecting backspaces
	int curr_curs_x, curr_curs_y; //for detecting backspaces
	int to_be_removed; //for storing number of chars to be removed after backspace

	fd_set set;
	FD_ZERO(&set);
	FD_SET(serial_port, &set);


	wmove(console1, LINES - 2, 1);
	//testy();
	
	boxes = div_init(10);
	draw_div_boxes(&boxes);

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
			print_div_byte(boxes.microsoft[boxes.print_pos], &boxes, incoming_byte);
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
