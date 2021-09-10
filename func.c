#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "header.h"
#include <sys/select.h>

extern struct div_disp boxes;
extern int errfile;

void init_curses() {
	initscr();
	cbreak();
	noecho();


	refresh();
}
WINDOW *create_window_box(int height, int width, int starty, int startx) {
	WINDOW *local_win;
	local_win = newwin(height, width, starty, startx);
	box(local_win, 0, 0);
	wrefresh(local_win);
	return local_win;
}
WINDOW *create_window_nobox(int height, int width, int starty, int startx) {
	WINDOW *local_win;
	local_win = newwin(height, width, starty, startx);
	wrefresh(local_win);
	return local_win;
}
void print_div_byte(WINDOW *console, struct div_disp *input, int a) {
	char *str = convert_byte_to_str(a);

	wattroff(input->microsoft[input->print_pos == 0 ? input->div - 1 : input->print_pos - 1], A_REVERSE);
	wattron(input->microsoft[input->print_pos == 0 ? input->div - 1 : input->print_pos - 1], COLOR_PAIR(4));
	mvwprintw(
			input->microsoft[(input->print_pos == 0 ? input->div - 1 : input->print_pos - 1)], 
			1, 
			1,
			"BYTE %d", (input->print_pos == 0 ? input->div - 1 : input->print_pos - 1));
	wrefresh(input->microsoft[input->print_pos == 0 ? input->div - 1 : input->print_pos - 1]);
	wattron(input->microsoft[input->print_pos], COLOR_PAIR(3));
	mvwprintw(
			input->microsoft[input->print_pos], 
			2, 
			1,
			"%s", str);
	wattron(input->microsoft[input->print_pos], COLOR_PAIR(4) | A_REVERSE);
	mvwprintw(
			input->microsoft[input->print_pos], 
			1, 
			1,
			"BYTE %d", input->print_pos);
	wattroff(input->microsoft[input->print_pos], A_REVERSE);
	free(str);
	wrefresh(input->microsoft[input->print_pos]);
	input->print_pos = (input->print_pos + 1 == input->div ? 0 : input->print_pos + 1);
}
char *convert_byte_to_str(int byte) {
	char *output = (char *) malloc(9 * sizeof(char));
	for(int i = 0; i < 8; i++) {
		if(byte & (1 << 7 - i)) {
			output[i] = '1';
			continue;
		}
		output[i] = '0';
	}
	return output;
}
int check_allowed_char(char c) {
	if(c < 32 && c != '\n') {
		return 0; //not allowed
	}
	return 1; //allowed
}

struct div_disp div_init(int div) { 
	int number = 0;
	//function to initialize variables to do with div-ing
	//and perhaps draw the boxes.
	struct div_disp output;
	output.div = div;
	output.print_pos = 0; //reset printing position
	
	output.div_num_y = ((LINES - 2 * DIV_PADDING_Y - 1) / 4);
	output.div_num_x = ((COLS/2 - 2 * DIV_PADDING_X - 1) / 10);
	
	output.microsoft = (WINDOW **) malloc(output.div_num_y * output.div_num_x * sizeof(WINDOW *));

	
	//allocation
	output.box_coords_yx = (int ***) malloc(output.div_num_y * sizeof(int **));
	for(int i = 0; i < output.div_num_y; i++) {
		output.box_coords_yx[i] = (int **) malloc(output.div_num_x * sizeof(int *));
		for(int k = 0; k < output.div_num_x; k++) {
			output.box_coords_yx[i][k] = (int *) malloc(2 * sizeof(int));
			if(i * output.div_num_x + k >= div) {
				goto loop_end; //break out of both loops
			}
		}
	}
loop_end:

	output.div_val = (char *) malloc(div * sizeof(char));

	//setting values
	for(int i = 0; i < output.div_num_y; i++) {
		for(int k = 0; k < output.div_num_x ; k++) {
			output.box_coords_yx[i][k][0] = DIV_PADDING_Y + 1 + 4*i;
			//((LINES - 2*DIV_PADDING_Y - 2) * i) / div_num_y ;//y coordinate of boxes (k,i)
			output.box_coords_yx[i][k][1] = (COLS / 2) + DIV_PADDING_X + 1 + 10*k; //x coordinate of boxes (k,i)
			if(i * output.div_num_x + k >= div) {
				return output;
			}
		}	
	}
	return output;
}
void draw_div_boxes(struct div_disp *input) {
	int number = 0;
	//drawing boxes
	for(int i = 0; i < input->div_num_y && number < input->div; i++) {

		for(int k = 0; k < input->div_num_x && number < input->div; k++) {
			input->microsoft[input->div_num_x * i + k] = create_window_box(4, 10, input->box_coords_yx[i][k][0], input->box_coords_yx[i][k][1]);
			wattron(input->microsoft[input->div_num_x * i + k], COLOR_PAIR(4) | A_BOLD);
			mvwprintw(input->microsoft[input->div_num_x * i + k], 1, 1, "Byte %d", input->div_num_x * i + k);
			wattron(input->microsoft[input->div_num_x * i + k], COLOR_PAIR(3) | A_BOLD);

			wrefresh(input->microsoft[input->div_num_x * i + k]);

			number++;
		}
	}
}
void free_stuff(struct div_disp *input) {
	for(int i = 0; i < input->div_num_y; i++) {
		for(int k = 0; k < input->div_num_x; k++) {
			char *error = (char *) malloc(25* sizeof(char));
			sprintf(error, "%d, %d\n", i, k);
			log_error(error);
			free(input->box_coords_yx[i][k]);
			wclear(input->microsoft[input->div_num_x * i + k]);
			wrefresh(input->microsoft[input->div_num_x * i + k]);
			delwin(input->microsoft[input->div_num_x * i + k]);//I think this leaves 1 unfreed but eh
			if((input->div_num_x) * i + k >= input->div) {
				goto free_loop_end;
			}
		}
		free(input->box_coords_yx[i]);
	}
free_loop_end:
	free(input->box_coords_yx);
	free(input->microsoft);
}

void print_byte(WINDOW *console, int byte, int print_point_y, int print_point_x) {
	wmove(console, print_point_y, print_point_x);
	for(int i = 0; i < 8; i++) {
		if(byte & (1 << 7 -i)) {
			waddch(console, '1');
		}
		else {
			waddch(console, '0');
		}
	}
}
/* DEPRECATED:
void read_routine(WINDOW *write_window, int fd) {
	unsigned int incoming_byte;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 10;

	fd_set set;
	FD_ZERO(&set);
	FD_SET(fd, &set);
	if(select(8, &set, NULL, NULL, &timeout)) {
		read(fd, &incoming_byte, 1);
		wprintw(write_window, "%u\n", incoming_byte);
		wrefresh(write_window);
	}
	return;
}*/
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
		exit_properly();
	}
	else if(!strncmp(input, "div ", 4)) {
		int tempdiv = strtoul(input + 4, NULL, 10);
		if(tempdiv == 0 || tempdiv > 64) {
			wprintw(console, "invalid number of divs\n");
			wrefresh(console);
			return;
		}
		*boxes = div_reinit(boxes, tempdiv);
	}	
	else if (!strncmp(input, "put", 3)) {
		char write_byte;
		char confirm = 0;
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
struct div_disp div_reinit(struct div_disp *input, int new_div) {
	//free existing memory and reallocate
	//probably inefficient af but I've done exactly fuck-all OS dev stuff 
	//so I don't know
	free_stuff(input);
	struct div_disp output;
	output = div_init(new_div);
	draw_div_boxes(&output);
	return output;
}
void destroy_window(WINDOW *window) {
	box(window, ' ', ' ');
	delwin(window);
}
void log_error(char *error) {
	write(errfile, error, strlen(error));
	
}

	
