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
WINDOW *create_window_box(int height, int width, int starty, int startx) 
{
	WINDOW *local_win;
	local_win = newwin(height, width, starty, startx);
	box(local_win, 0, 0);
	wrefresh(local_win);
	return local_win;
}
WINDOW *create_window_nobox(int height, int width, int starty, int startx) 
{
	WINDOW *local_win;
	local_win = newwin(height, width, starty, startx);
	wrefresh(local_win);
	return local_win;
}
//Prints a byte in a div box, handling conversion
void print_div_byte(WINDOW *console, struct div_disp *input, uint8_t a) 
{
	log_error("Now trying to print byte %d at print_pos %d\n", a, input->print_pos);
	input->val[input->print_pos] = a;
	//handles conversion and prints to box
	if (input->uinit_boxes) {
		log_error("Initializing box %d. %d uninitialized boxes remaining", input->print_pos, input->uinit_boxes);
		input->uinit_boxes--; //one less to initialize
		bprint("        ", *input, console, input->print_pos); //clear the box
	}
	bprint(convert_to_base(a, input->base), *input, console, input->print_pos);	

	input->print_pos = input->print_pos >= input->div - 1 ? 0 : input->print_pos + 1;
}
void div_realloc(struct div_disp *input, int new_div)
{
	int new_blockno = DIV_DISP_BLOCKSIZE * (new_div / DIV_DISP_BLOCKSIZE + 1);
	input->val = realloc(input->val, new_blockno * DIV_DISP_BLOCKSIZE * sizeof(uint8_t));
}
char *convert_to_base(uint8_t byte, int base)
{
	//I could implement a proper algo, but I am lazy af so I do it this shitty way
	
	//Well actually... what's so shitty about this? I don't imagine
	//using asprintf is so bad...
	char *output; 

	//0 - binary
	//1 - octal
	//2 - decimal
	//3 - hex
	
	//I guess this is where you'd use an enum in c++

	switch (base) {
		case 0:
			//we actually gotta do work here
			output = convert_to_base_two(byte);
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
//Does the actual printing
void bprint(char *str, struct div_disp input, WINDOW *console, int box_no)
{
	if (!check_in_bounds(input, box_no)) {
		log_error("Tried to write to box outside of printable range (p_range)!\n");
		return;
	}
	if (strlen(str) > BOX_WIDTH - 2) {
		log_error("BOX_WIDTH exceeded when printing!\n");
		bprint("ERROR", input, console, box_no); //recursion!
		return;
	}
	mvwprintw(console, get_box_y(input, box_no) + 2, get_box_x(input, box_no) + 1, str);
	wrefresh(console);
}
char *convert_to_base_two(uint8_t byte) 
{
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
	output.uinit_boxes = div; //all boxes uninitialized
	output.offset = 0;
	output.base = 2;
	
	output.val = malloc(div * DIV_DISP_BLOCKSIZE * sizeof(uint8_t));
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
	//gets range of printable box indexes in the div_disp struct.
	//Keep in mind that the bounds are inclusive - the lo_bound and hi_bound
	//correspond to the index of the lowest and highest printable boxes respectively.
	struct p_range output;
	output.lo_bound = boxes.offset;
	output.hi_bound = boxes.div - boxes.offset > max_boxes(boxes) ? boxes.offset + max_boxes(boxes) - 1 : boxes.div - 1 - boxes.offset;
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
	int onscreen_boxes = get_p_box_no(input);
	log_error("on-screen boxes:%d\n", onscreen_boxes);
	for (int i = 0; i < onscreen_boxes; i++) {
		log_error("printing box %d!\n", i);
		enbox(input, console, i);
	}
	//'initialize' all the values:
	for (int i = 0; i < onscreen_boxes; i++) {
		bprint("NO VALUE", input, console, i);
	}	
	wrefresh(console);
}
void redraw_div_boxes(int new_div, struct div_disp *input, WINDOW *console)
{
	//what I left on:
	//This function at present doesn't account for various potential shifts, it seems.
	//Particularly in the uinit_boxes != 0 part. Once I fix that, I just need to make sure
	//the reinit function handles stuff correctly and I'll be set! I'll just do some testing
	//and then reconnect the console with the additional commands and reworked original commands
	//and the big patch will be done!
	//After that I can finally stop developing this tool for a while and shift back to the cam (which 
	//I assume will be quite easy to get up (assuming also that I'm careful not to apply excessive voltage
	//to it again...)

	werase(console);
	div_reinit(input, new_div);
	if (!input->cool) {
		for (int i = input->offset; i < min(input->div, input->offset + max_boxes()); i++) {
			enbox(*input, console, i);
		}
	}
	else {
		for (int i = 0; i < min(input->div, input->offset + max_boxes()); i++) {
			cool_enbox(*input, console, i);
		}
	}
	if (input->uinit_boxes == 0) {
		//simply draw the visible values
		for (int i = input->offset; i < min(input->div, input->offset + max_boxes()); i++) {
				bprint(convert_to_base(input->val[i], input->base), *input, console, i);
		}
	}
	else if (input->div - input->uinit_boxes == 0) {//if we don't check for this special case,
		//the code that follows, which uses print_pos, will possibly have unintented behaviour
		for (int i = input->offset; i < min(input->div, input->offset + max_boxes()); i++) {
				bprint("NO VALUE", *input, console, i);
		}

	} 
	else { 
		//The way this is handled could be too complex. for simpler ideas
		//for potential future reworks, see the new_div < input->div section
		//of the div_reinit function (untested at time of writing...)
		struct p_range uinit_boxes;
		struct p_range vis_boxes;
		int switches = 0; //Number of times the for loop should switch between printing uinit'd and init'd boxes
		int mode = 0; //Current printing mode. 0 for uinit, 1 for init
		int nexts = 0; //Next switch index
		int prevs = input->offset; //Last switch index

		uinit_boxes.lo_bound = input->print_pos; //Inclusive
		uinit_boxes.hi_bound = (input->print_pos + input->uinit_boxes - 1) % input->div;		
		vis_boxes.lo_bound = input->offset;
		vis_boxes.hi_bound = min(input->offset + max_boxes(), input->div);

		if (is_in_range(input->print_pos, vis_boxes, *input)) {
			switches++;
		}
		if (is_in_range(input->print_pos + input->uinit_boxes + 1, vis_boxes, *input)) {
			switches++;
		}
		if (is_in_range(vis_boxes.lo_bound, uinit_boxes, *input))
			mode = 0; //Start with uinit boxes
		else
			mode = 1; //Start with init boxes
		nexts = next_switch(mode, uinit_boxes, vis_boxes, *input);
		for (int k = 0; k < switches + 1; k++) {
			if (mode) {
				for(int i = prevs; i < nexts; i++) {
					bprint(convert_to_base(input->val[i], input->base), *input, console, i);
				}	
			}
			else {
				for (int i = prevs; i < nexts; i++) {
					bprint("NO VALUE", *input, console, i);
				}
			}
			mode = !mode;
			prevs = nexts;
			nexts = next_switch(mode, uinit_boxes, vis_boxes, *input);
		}
	}

}
int next_switch(int mode, struct p_range uinit_boxes, struct p_range vis_boxes, struct div_disp input) {
	//Calling this function more times than there are switches results in undefined
	//behaviour due to return values possibly being outside visible range.
	
	if (mode) //We are finding the switch init -> uinit
		return min(uinit_boxes.lo_bound,  vis_boxes.hi_bound); 
	else //We are finding the switch uinit -> init
		return min(uinit_boxes.hi_bound + 1, vis_boxes.hi_bound);
}
int mulmin(int argc, ...) //argc does not include itself in the count
{
	va_list args;
	int tmin; //Temporary minimum
	int temp; //For storing next arg

	va_start(args, argc);
	tmin = va_arg(args, int);
	
	for (int i = 0; i < argc - 1; i++) {
		temp = va_arg(args, int);
		if (temp < tmin)
			tmin = temp;
	}
	va_end(args);
	return tmin;
}
int is_in_range(int num, struct p_range range, struct div_disp input)
{
	//Checks assuming bounds inclusive
	if ((num - range.lo_bound) % input.div > (range.hi_bound - range.lo_bound) % input.div)
		return 0;
	return 1;
}
int is_uinit(int box_no, struct div_disp input)
{
	struct p_range uinit_boxes();
	//Stopped here. This still doesn't take into account the possibility that
	//the init'd boxes are surrounded by uinit_boxes. Particularly because of the 
	//1st condition...
	if (box_no >= input.print_pos && box_no - input.print_pos < input.uinit_boxes)
		return 1;
	return 0;
}
int get_p_box_no(struct div_disp input)
{
	//gets number of printable boxes on right terminal
	struct p_range temp = get_p_boxes_range(input);
	return temp.hi_bound - temp.lo_bound + 1;
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
void cool_enbox(struct div_disp boxes, WINDOW *console, int box_no)
{
	log_error("making box with coords %d,%d\n", get_box_y(boxes, box_no), get_box_x(boxes, box_no));
	//we cannot use routines such as hline or box which would speed this up
	//since box works with windows (and maybe sub-windows but I didn't know about
	//this at the time) and xline routines use waddch which doesn't work for
	//utf-8 characters, I think...
	

	//some utf-8 characters
	//the letters t, b, l, r, s stand for top, bottom, left, right and side respectively
	char *tl = "\u256d";
	char *tr = "\u256e";
	char *bl = "\u2570";
	char *br = "\u256f";
	char *ls = "\u2502";
	char *rs = "\u2502";
	char *ts = "\u2501";
	char *bs = "\u2501";

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
void free_stuff(struct div_disp *input) 
{
	free(input->val);
}
void div_reinit(struct div_disp *input, int new_div) 
{
	if (new_div > input->div) {
		input->uinit_boxes += new_div - input->div; //mark new boxes as uninitialized
		input->div = new_div;
		div_realloc(input, new_div);
	}
	if (new_div < input->div) {

		if (input->print_pos > new_div) {
			input->print_pos = 0; //if we shrink past the current printpos, we start at beginning
		}
		if (new_div > input->print_pos) {
			input->uinit_boxes += new_div - input->div;
		}
		if (new_div < input->print_pos) {
			int sub = 0; //how many uinit_boxes to subtract
			struct p_range uinit_boxes;
			struct p_range d_range; //lol... range of boxes to be destroyed
			uinit_boxes.lo_bound = input->print_pos;
			uinit_boxes.hi_bound = (input->print_pos + input->uinit_boxes - 1) % input->div;
			d_range.lo_bound = new_div;
			d_range.hi_bound = input->div;
			
			if (uinit_boxes.lo_bound <= uinit_boxes.hi_bound) {
				log_error("uinit_boxes.lo_bound <= uinit_boxes.hi_bound");
				sub += uinit_boxes.hi_bound > d_range.lo_bound
					? uinit_boxes.hi_bound - d_range.lo_bound + 1 
					: 0;
			}
			else {
				log_error("uinit_boxes.lo_bound > uinit_boxes.hi_bound");
				sub += uinit_boxes.hi_bound > d_range.lo_bound
					? uinit_boxes.hi_bound - d_range.lo_bound + 1
					: 0;
				sub += d_range.hi_bound - uinit_boxes.lo_bound;
			}
			if (sub > input->uinit_boxes || sub > input->div - new_div) {
				log_error("Shrinked (shrunk?) number of uinit_boxes to be subbed\n");
				sub = min(input->uinit_boxes, input->div - new_div);
			}
			input->uinit_boxes -= sub;
		}
		input->div = new_div;
		div_realloc(input, new_div);
	}

}
void shift_val(struct div_disp *input, int shift_by)
{
	uint8_t temp[input->div];
	memcpy(temp, input->val, input->div);
	for (int i = 0; i < input->div; i++) {
		input->val[i] = temp[(i + shift_by) % input->div];
	}
}
//Yeah yeah implementing functions that have little to do with this library
//in particular seems like bad practice...
int min(int a, int b)
{
	return (a < b) ? a : b;
}
int max(int a, int b) {
	return (a > b) ? a : b;
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
