#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "curses_func.h"
#include <sys/select.h>

//functions to be added to interpret:
//1. shift
//2. base
//3. clear right
//4. scroll (though the arrow keys should normally suffice

extern struct div_disp boxes;
extern int errfile;

void exit_prep();
//void exit_properly(int ass);

int check_allowed_char(char c) 
{
	if(c < 32 && c != '\n') {
		return 0; //not allowed
	}
	return 1; //allowed
}
struct args parargs(char *argstr)
{
	log_error("parsing args\n");
	struct args out;
	int count;
	out.argv = malloc(2 * sizeof(char *)); //start with 2 cos why not
	count = 2; //number of allocated pointers
	

	out.argv[0] = strtok(argstr, " ");
	out.argc = 0;
	for (int i = 1; out.argv[i - 1] != NULL; i++) {
		out.argv[i] = strtok(NULL, " ");
		out.argc++;
		log_error("argc: %d\n", out.argc);
		if (out.argc == count) {
			log_error("reallocating interpret argv\n");
			out.argv = realloc(out.argv, 2 * sizeof(char *) * count);
			count *= 2;
		}
	}
	log_error("done parsing\n");
	return out;
}

void interpret(char *input, WINDOW *console, int fd, struct div_disp *boxes, WINDOW *console2) 
{ //fd for serial port
	struct args argu;
	argu = parargs(input);
	log_error("interpreting!\n");
	if(!strcmp(argu.argv[0], "help")) {
		wprintw(console, "Mini-terminal commands:\n");

		comprint("put <byte0> <byte1> ...", "attempt to write specified bytes", console);

		comprint("clear [left|right|all]", "clear the left window", console);

		comprint("div <d>", "divide input into <d> divisions", console);

		comprint("quit/exit", "close the program", console);

		comprint("help", "display this help message", console);

		comprint("shift [-]<n>", "shift the byte array n bytes to the left (if with \"-\") or right", console);

		comprint("base [bin|oct|dec|hex]", "print received bytes in the specified base", console);
	}
	else if(!strcmp(argu.argv[0], "clear")) {
		if (argu.argc != 2) {
			wprintw(console, "Invalid number of arguments. See \"help\".");
			free(argu.argv);
			return;
		}
		if (!strcmp(argu.argv[1], "left")) {
			werase(console);
			wrefresh(console);
			free(argu.argv);
			return;
		} //right to be implemented
		else {
			wprintw(console, "invalid args. See \"help\".");
			free(argu.argv);
			return;
		}
	}
	else if (!strcmp(argu.argv[0], "quit") || !strcmp(argu.argv[0], "exit")) {
		exit_prep();
		endwin();
		fprintf(stderr, "user quit\n");
		exit(0);
	}
	else if(!strcmp(argu.argv[0], "div")) {
		if (argu.argc != 2) {
			wprintw(console, "invalid number of args\n");
			wrefresh(console);
			free(argu.argv);
			return;
		}
		int tempdiv = strtoul(argu.argv[1], NULL, 10);
		if(tempdiv == 0 || tempdiv > 64) {
			wprintw(console, "invalid number of divs\n");
			wrefresh(console);
			free(argu.argv);
			return;
		}
		redraw_div_boxes(tempdiv, boxes, console2);
	}	
	else if (!strcmp(argu.argv[0], "put")) {
		char write_byte;
		for (int i = 0; i < argu.argc - 1; i++) {
			write_byte = strtoul(argu.argv[i + 1], NULL, 2);
			wprintw(console, "Gonna print byte %u\n", write_byte);
			write(fd, &write_byte, 1);
		}
	}
	else if (!strcmp(argu.argv[0], "test")) {
		print_div_byte(console2, boxes, (uint8_t) 20);
	}
	else {
		wprintw(console, "command %s unknown. try \"help\"\n", argu.argv[0]);
		free(argu.argv);
		return;
	}
}
void comprint(char *com, char *str, WINDOW *console)
{
	char *desc = malloc(strlen(str) + 4);
	//Prints command and command description. for "help" command
	wattr_on(console, A_REVERSE, NULL);
	wprintw(console, com);
	wattr_off(console, A_REVERSE, NULL);
	sprintf(desc, " - ");
	desc = strcat(desc , str);
	desc = strcat(desc, "\n");

	wprintw(console, desc);
}
