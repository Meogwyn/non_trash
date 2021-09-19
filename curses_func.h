//padding of boxes in right console
#define DIV_PADDING_Y 3 
#define DIV_PADDING_X 3
#define WIDTH COLS/2
#define HEIGHT LINES
#define NOT_INITIALIZED 256
#define BOX_HEIGHT 4
#define BOX_WIDTH 10

struct div_disp {
	unsigned int div; //amount of divisions
	unsigned int print_pos;
	unsigned int offset; //for scrolling. Represents box to be printed at top left
	//of console2

	int *val;//for storing values, which we need for resizing the screen and stuff
	//bear in mind that val stores the values in already-converted format
	//hence we need to allocate 8 chars per div if in binary mode
	//2 if in hex and 3 if in dec.
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

//implement "shift" function that allows one to shift the indexes
//given bytes get printed to (in other words, to shift the printpos and reconstruct
//the existing boxes)

struct consoles init_curses();
WINDOW *create_window_box(int height, int width, int starty, int startx);
WINDOW *create_window_nobox(int height, int width, int starty, int startx);
void print_div_byte(WINDOW *console, struct div_disp *input, int byte);//-
char *convert_byte_to_str(int byte);
struct div_disp create_div_disp(int div);
void draw_div_boxes(struct div_disp input, WINDOW *console); //+
void enbox(struct div_disp input, WINDOW *console, int box_no);
void cool_enbox(struct div_disp input, WINDOW *console);
void free_stuff(struct div_disp *input); //+
struct div_disp div_reinit(struct div_disp *input, int new_div); //+
void log_error(char *format, ...);
struct p_range get_p_boxes_range(struct div_disp boxes);
int check_in_bounds(struct div_disp boxes, int index);
int get_box_y(struct div_disp boxes, int box_no); 
int get_box_x(struct div_disp boxes, int box_no);
int max_boxes();
int get_max_boxes_y();
int get_max_boxes_x();
void interpret(char *input, WINDOW *console, int fd, struct div_disp *boxes);
void testy(WINDOW *console, struct div_disp boxes);
