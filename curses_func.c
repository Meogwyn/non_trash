#define _GNU_SOURCE
#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <stdarg.h>
#include "curses_func.h"


extern struct div_disp boxes;
extern int errfile;

struct consoles init_curses() {

	struct consoles output;
	setlocale(LC_ALL, "");
	initscr();
	cbreak();
	noecho();
	refresh();

	int startx = 0;
	int starty = 0;

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
	wattron(output.console2, COLOR_PAIR(3) | A_BOLD);
	wprintw(output.console1, "Hello, World!\n");
	wprintw(output.console2, "Hello, World!\n");
	wrefresh(output.console1);
	wrefresh(output.console2);
	return output;
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
void print_div_byte(WINDOW *console, struct div_disp *input, uint8_t a) 
{
	input->val[input->print_pos] = a;
	//handles conversion and prints to box
	bprint(convert_to_base(a, input->base));	
	input->print_pos++;
	return;
}
char *convert_to_base(uint8_t byte, int base)
{
	//I could implement a proper algo, but I am lazy af so I do it this shitty way
	char *output; 

	//0 - binary
	//1 - octal
	//2 - decimal
	//3 - hex

	switch (base) {
		case 0:
			//we actually gotta do work here
			output = convert_byte_to_str(byte);
			return output;
		case 1:
			asprintf(&output, "0q%o", byte);
			return output;
		case 2:
			asprintf(&output, "%d", byte);
			return output;
		case 3:
			asprintf(&output, "0x%x", byte);
			return output;
	}
	log_error("...how did you get here?...\n");
	return NULL;
}
void bprint(char *str, struct div_disp input, WINDOW *console, int box_no)
{
	if (!check_in_bounds(input, box_no)) {
		log_error("Tried to write to box outside of printable range (p_range)");
		return;
	}
	if (strlen(str) > BOX_WIDTH - 2) {
		log_error("BOX_WIDTH exceeded when printing!\n");
		bprint("ERROR", input, console, box_no); //recursion!
		return;
	}
	mvwprintw(console, get_box_y(input, box_no) + 2, get_box_x(input, box_no) + 1, str);
}
char *convert_byte_to_str(uint8_t byte) {
	char *output = (char *) malloc(9 * sizeof(char));
	for(int i = 0; i < 8; i++) {
		if(byte & (1 << (7 - i))) {
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
	log_error("div when creating:%d\n", div);
	output.div = div;
	log_error("div value of struct:%d\n", output.div);
	output.print_pos = 0;
	output.offset = 0;
	output.base = 10;
	
	output.val = (uint8_t *) malloc(div * sizeof(uint8_t));
	return output;
}
/* Uncomment this when you implement the redraw functions cos we'll need 'em
void change_base (int base, struct div_disp *input) 
{
	if(base != 2 ||
	input->base = 
}*/

struct p_range get_p_boxes_range(struct div_disp boxes)
{
	//gets range of printable box indexes
	struct p_range output;
	output.lo_bound = boxes.offset;
	output.hi_bound = boxes.offset + max_boxes(boxes) - 1;
	return output;
}
int check_in_bounds(struct div_disp boxes, int index)
{
	struct p_range range = get_p_boxes_range(boxes);
	if (index >= range.lo_bound && index <= range.hi_bound)
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
	return DIV_PADDING_Y + BOX_HEIGHT * ((box_no - boxes.offset) / get_max_boxes_x());
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

void draw_div_boxes(struct div_disp input, WINDOW *console) 
{
	int onscreen_boxes = (max_boxes() < input.div) ? max_boxes : input.div;
	log_error("on-screen boxes:%d\n", onscreen_boxes);
	for (int i = 0; i < onscreen_boxes; i++) {
		log_error("printing box %d!\n", i);
		enbox(input, console, i);
	}
	//'initialize' all the values:
	for (int i = 0; i < onscreen_boxes; i++) {
		bprint("NO VALUE", input, console, i);
	}	
}
void enbox(struct div_disp boxes, WINDOW *console, int box_no)
{
	log_error("making box with coords %d,%d\n", get_box_y(boxes, box_no), get_box_x(boxes, box_no));
	//we cannot use routines such as hline or box which would speed this up
	//since box works with windows (and maybe sub-windows but I didn't know about
	//this at the time) and xline routines use waddch which doesn't work for
	//utf-8 characters, I think...
	

	//some utf-8 characters
	//the letters t, b, l, r, s stand for top, bottom, left, right and side respectively
	char *tl = "\xe2\x95\x94";
	char *tr = "\xe2\x95\x97";
	char *bl = "\xe2\x95\x9a";
	char *br = "\xe2\x95\x9d";
	char *ls = "\xe2\x95\x91";
	char *rs = "\xe2\x95\x91";
	char *ts = "\xe2\x95\x90";
	char *bs = "\xe2\x95\x90";

	//drawn in the same order as they were defined above
	mvwaddstr(console, get_box_y(boxes, box_no), get_box_x(boxes, box_no), tl);
	mvwaddstr(console, get_box_y(boxes, box_no), get_box_x(boxes, box_no) + BOX_WIDTH - 1, tr);
	mvwaddstr(console, get_box_y(boxes, box_no) + BOX_HEIGHT - 1, get_box_x(boxes, box_no), bl);
	mvwaddstr(console, get_box_y(boxes, box_no) + BOX_HEIGHT - 1, get_box_x(boxes, box_no) + BOX_WIDTH - 1, br);
	for (int i = 1; i < BOX_HEIGHT - 1; i++) {
		mvwaddstr(console, get_box_y(boxes, box_no) + i, get_box_x(boxes, box_no), ls);
	}
	for (int i = 1; i < BOX_HEIGHT - 1; i++) {
		mvwaddstr(console, get_box_y(boxes, box_no) + i, get_box_x(boxes, box_no) + BOX_WIDTH - 1, rs);
	}
	for (int i = 1; i < BOX_WIDTH - 1; i++) {
		mvwaddstr(console, get_box_y(boxes, box_no), get_box_x(boxes, box_no) + i, ts);
	}
	for (int i = 1; i < BOX_WIDTH - 1; i++) {
		mvwaddstr(console, get_box_y(boxes, box_no) + BOX_HEIGHT - 1, get_box_x(boxes, box_no) + i, bs);
	}
}
void cool_enbox(struct div_disp boxes, WINDOW *console)
{
}
void free_stuff(struct div_disp *input) {
}
struct div_disp div_reinit(struct div_disp *input, int new_div) {
}
void log_error(char *format, ...) 
{
	va_list args;
	va_start(args, format);
	vdprintf(errfile, format, args);
	va_end(args);
	return;
}
void testy(WINDOW *console, struct div_disp boxes)
{
	log_error("div: %d\n", boxes.div);
	log_error("the const:%d", NCURSES_WIDECHAR);
	draw_div_boxes(boxes, console);
	wrefresh(console);
	while (1) {
	}
	return;
}
