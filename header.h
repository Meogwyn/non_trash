//padding of boxes in right console
#define DIV_PADDING_Y 3 
#define DIV_PADDING_X 3

struct div_disp {
	unsigned int div; //amount of divisions
	int print_pos;
	int div_num_y; //number of divs in y axis during display
	int div_num_x; //number of divs in x axis during display

	char *div_val; //value of nth byte in division (can be larger than following display arrrays)

	int ***box_coords_yx; //coords of first char of nth box
	
	WINDOW **microsoft; //get it?
};

void init_curses();
WINDOW *create_window_box(int height, int width, int starty, int startx);
WINDOW *create_window_nobox(int height, int width, int starty, int startx);
void print_div_byte(WINDOW *console, struct div_disp *input, int byte);
char *convert_byte_to_str(int byte);
int check_allowed_char(char c);
struct div_disp div_init(int div);
void draw_div_boxes(struct div_disp *input);
void free_stuff(struct div_disp *input);
void print_byte(WINDOW *console, int byte, int print_point_y, int print_point_x);
void read_routine(WINDOW *write_console, int fd);
void interpret(char *input, WINDOW *console, int fd, struct div_disp *boxes);
void destroy_window();
struct div_disp div_reinit(struct div_disp *input, int new_div);
void exit_properly();
void log_error(char *error);
