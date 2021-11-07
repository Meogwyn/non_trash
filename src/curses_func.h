//padding of boxes in right console
#define DIV_PADDING_Y 3 
#define DIV_PADDING_X 3
#define WIDTH COLS/2
#define HEIGHT LINES
//#define NOT_INITIALIZED 256
#define BOX_HEIGHT 4
#define BOX_WIDTH 10

#define DIV_DISP_BLOCKSIZE 64

struct div_disp {
	int div; //amount of divisions
	int print_pos; //the box_no we're printing to
	int offset; //for scrolling. Represents box to be printed at top left
	int uinit_boxes; //number of uninitialized boxes remaining. 
	int shift; //in order to keep track of shifting. We wouldn't need this if I didn't have NO VALUE bytes
	unsigned int base : 2; //base of byte representations
	unsigned int cool : 1;

	uint8_t *val; //stores values of received bytes in unconverted form.
};
struct consoles {
	WINDOW *console1;
	WINDOW *console2;
	WINDOW *console1_box;
	WINDOW *console2_box;
};
struct p_range {
	int lo_bound;
	int hi_bound;
};
struct args {
	char **argv;
	int argc;
};

//implement "shift" function that allows one to shift the indexes
//given bytes get printed to (in other words, to shift the printpos and reconstruct
//the existing boxes)

//An idea for later: perhaps include console2 into the div_disp struct? 
//Seems like lots of argument lists would be reduced and the functions
//themselves made better for the (unlikely) potential user.

//New functions:

//Side notes:
//1. When implementing the signal handler for resizing the terminal, make 
//sure the user has an option to disable that, as possibly performance could
//suffer from continuous redrawing or something...
//2. Don't forget the cool

void add_div_boxes(struct div_disp input, WINDOW *console, int n);
void remove_div_boxes(struct div_disp input, WINDOW *console, int n);

//---curses_func.c

struct consoles init_curses();
WINDOW *create_window_box(int height, int width, int starty, int startx);
WINDOW *create_window_nobox(int height, int width, int starty, int startx);
void print_div_byte(WINDOW *console, struct div_disp *input, uint8_t byte);//-
char *convert_to_base(uint8_t byte, int base);
void div_realloc(struct div_disp *input, int new_div);
void bprint(char *str, struct div_disp input, WINDOW *console, int box_no);
char *convert_to_base_two(uint8_t byte);
struct div_disp create_div_disp(int div);
void draw_div_boxes(struct div_disp input, WINDOW *console); //+
void redraw_div_boxes(int new_div, struct div_disp *input, WINDOW *console);
int next_switch(int mode, struct p_range uinit_boxes, struct p_range vis_boxes, struct div_disp input);
int mulmin (int argc, ...);
int is_in_range(int num, struct p_range range, struct div_disp input);
int is_uinit(int box_no, struct div_disp input);
int get_p_box_no(struct div_disp input);
void change_base(struct div_disp *input);
void enbox(struct div_disp input, WINDOW *console, int box_no);
void cool_enbox(struct div_disp input, WINDOW *console, int box_no);
void free_stuff(struct div_disp *input); //+
void div_reinit(struct div_disp *input, int new_div); //+
void copy_shrink_val(struct div_disp *input);
int min(int a, int b);
int max(int a, int b);
void log_error(char *format, ...);
struct p_range get_p_boxes_range(struct div_disp boxes);
int check_in_bounds(struct div_disp boxes, int index);
int get_box_y(struct div_disp boxes, int box_no); 
int get_box_x(struct div_disp boxes, int box_no);
int max_boxes();
int get_max_boxes_y();
int get_max_boxes_x();
void testy(WINDOW *console, struct div_disp boxes);
void div_redraw(struct div_disp input, int new_div);

//---func.c

int check_allowed_char(char c);
struct args parargs(char *argstr);
void expandus(char ***argv, int ccount);
void interpret(char *input, WINDOW *console, int fd, struct div_disp *boxes, WINDOW *console2); //fd for serial port
void comprint(char *com, char *str, WINDOW *console);
