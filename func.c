#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "header.h"
#include <sys/select.h>

extern struct div_disp boxes;
extern int errfile;
//void exit_properly(int ass);

int check_allowed_char(char c) {
	if(c < 32 && c != '\n') {
		return 0; //not allowed
	}
	return 1; //allowed
}

void interpret(char *input, WINDOW *console, int fd, struct div_disp *boxes) { //fd for serial port
	if(!strcmp(input, "help")) {
		wprintw(console, "Mini-terminal commands:\n");

		wattr_on(console, A_REVERSE, NULL);
		wprintw(console, "put <byte0> <byte1> ...");
		wattr_off(console, A_REVERSE, NULL);
		
		wprintw(console, " - attempt to write the specified bytes\n");
		
		wattr_on(console, A_REVERSE, NULL);
		wprintw(console, "clear [left|right]");
		wattr_off(console, A_REVERSE, NULL);

		wprintw(console, " - clear the left window\n");

		wattr_on(console, A_REVERSE, NULL);
		wprintw(console, "div <d>");
		wattr_off(console, A_REVERSE, NULL);

		wprintw(console, "- divides input into <d> divisions\n");

		wattr_on(console, A_REVERSE, NULL);
		wprintw(console, "quit/exit");
		wattr_off(console, A_REVERSE, NULL);
		
		wprintw(console, "- close the program\n");

		wattr_on(console, A_REVERSE, NULL);
		wprintw(console, "help");
		wattr_off(console, A_REVERSE, NULL);

		wprintw(console, " - display this help message\n");
	}
	else if(!strncmp(input, "clear", 5)) {
		if(!strncmp(input + 6, "left", 4)) {
			werase(console);
			wrefresh(console);
			return;
		}	
		else {
			wprintw(console, "invalid args. See \"help\"");
			return;
		}
	}
	else if (!strcmp(input, "quit") || !strcmp(input, "exit")) {
		//exit_properly(1);
	}
	else if(!strncmp(input, "div ", 4)) {
		int tempdiv = strtoul(input + 4, NULL, 10);
		if(tempdiv == 0 || tempdiv > 64) {
			wprintw(console, "invalid number of divs\n");
			wrefresh(console);
			return;
		}
		//*boxes = div_reinit(boxes, tempdiv);
	}	
	else if (!strncmp(input, "put", 3)) {
		char write_byte;
		input = strtok(input, " ");

		input = strtok(NULL, " ");
		while(input != NULL) {
			
			write_byte = strtoul(input, NULL, 2);
			wprintw(console, "Gonna print byte %u\n", write_byte);
			
			/*
			wprintw(console, "Gonna print byte %u. Confirm? (y/n)\n", write_byte);
			while(1) {
				confirm = wgetch(console);
				if(confirm != ERR) {
					if(confirm != 'y') {
						wprintw(console, "Cancelled!\n");
						return;
					}
					wprintw(console, "Confirmed!\n");
					write(fd, &write_byte, 1);
					break;
				}
				read_routine(console2, fd);
			}
			*/
				
			write(fd, &write_byte, 1);
			input = strtok(NULL, " ");
		}
	}
	else {
		wprintw(console, "command %s unknown. try \"help\"\n", input);
		return;
	}
}
