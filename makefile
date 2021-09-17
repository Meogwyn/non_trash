CFLAGS = -lncurses
obj = main.o func.o curses_func.o serial_func.o

.PHONY: clean

main.o:header.h curses_func.h serial_func.h
func.o:header.h
curses_func.o:curses_func.h
serial_func.o:serial_func.h


clean:
	-rm -v main $(obj) errorfile
