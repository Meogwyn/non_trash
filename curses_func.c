#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include "curses_func.h"

extern struct div_disp boxes;
extern int errfile;

struct consoles init_curses() {

	struct consoles output;
	initscr();
	cbreak();
	noecho();
	refresh();

	int startx = 0;
	int starty = 0;
	char *user_input = (char *) calloc(1024, sizeof(char));
	int char_count = 0;

	//Configure windows
	

	output.console1_box = create_window_box(HEIGHT, WIDTH, starty, startx);	
	
	startx = COLS/2;

	output.console2_box = create_window_box(HEIGHT, WIDTH, starty, startx);
	
	startx = 1;
	starty = 1;

	output.console1 = create_window_nobox(HEIGHT - 2, WIDTH - 2, starty, startx);
	scrollok(output.console1, TRUE);
	idlok(output.console1, TRUE);
	wsetscrreg(output.console1, 1, HEIGHT - 2);
	nodelay(output.console1, TRUE);
	curs_set(0);

	startx += COLS/2;

	
	output.console2 = create_window_nobox(HEIGHT - 2, WIDTH - 2, starty, startx);
	//scrollok(output.console2, TRUE);
	//idlok(output.console2, TRUE);
	//wsetscrreg(output.console2, 1, HEIGHT - 2);
	

	//Deal with color:
	start_color();
	
	init_pair(1, COLOR_YELLOW, COLOR_BLACK); //left terminal user text
	init_pair(2, COLOR_RED, COLOR_BLACK); //left terminal response text
	init_pair(3, COLOR_GREEN, COLOR_BLACK); //right terminal readback text
	init_pair(4, COLOR_CYAN, COLOR_BLACK); //right terminal readback text

	wattron(output.console1, COLOR_PAIR(1) | A_BOLD);
	//wattron(output.console2, COLOR_PAIR(3) | A_BOLD);
	wprintw(output.console1, "Hello, World!\n");
	wprintw(output.console2, "Hello, World!\n");
	wrefresh(output.console1);
	wrefresh(output.console2);
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

struct div_disp create_div_disp(int div) 
{ 
	struct div_disp output;
	output.div = div;
	output.print_pos = 0;/r
	output.offset = 0;
	
	output.val = (char *) malloc(div * sizeof(int));
	for(int i = 0; i < output.div; i++) {
		output.val[i] = NOT_INITIALIZED; //just outside reasonable range
	}
}
struct p_range get_p_boxes_range(struct div_disp boxes)
{
	//gets range of printable box indexes
	struct p_range output;
	output.lo_bound = boxes.offset;
	output.hi_bound = boxes.offset + max_boxes(boxes) - 1;
}
int check_in_bounds(struct div_disp boxes, int index)
{
	struct p_range range = get_p_boxes_range(boxes);
	if (index >= lo_bound && index <= hi_bound)
		return 1; //in bounds
	else
		return 0; //out of bounds!
}
//ATTENTION! the following 2 functions now return the top left corner of the box
//as opposed to the first character, as was the way in the previous implementation
//(rip)
int get_box_y(struct div_disp boxes, int box_no)
{
	if (!check_in_bounds(boxes, box_no)) {
		return -1; //error!
	}
	return DIV_PADDING_Y + BOX_HEIGHT * (box_no - boxes.offset) / get_max_boxes_y();
}	
int get_box_x(struct div_disp boxes, int box_no)
{
	if (!check_in_bounds(boxes, box_no)) {
		return -1; //error!
	}
	return DIV_PADDING_X + BOX_WIDTH * (box_no % get_max_boxes_x());
}
int max_boxes()
{
	return get_max_boxes_y() * get_max_boxes_x();
}
int get_max_boxes_y()
{
	return (LINES - 2 * DIV_PADDING_Y) / BOX_HEIGHT;
}
int get_max_boxes_x()
{
	return (COLS / 2 - 2 * DIV_PADDING_X) / BOX_WIDTH;
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
void log_error(char *error) {
}
