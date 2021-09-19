CFLAGS = -lncurses -Wall
obj = main.o func.o curses_func.o serial_func.o

.PHONY: clean

main:$(obj)
	cc $(CFLAGS) $(obj) -o main

main.o:header.h curses_func.h serial_func.h
func.o:header.h
curses_func.o:curses_func.h
serial_func.o:serial_func.h

valgr:debug_comp
	-rm -vf valgr_log.txt
	touch valgr_log.txt
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all --log-file=valgr_log.txt ./main

debug_comp:clean
	cc -g -c $(CFLAGS) -o main.o main.c 
	cc -g -c $(CFLAGS) -o func.o func.c 
	cc -g -c $(CFLAGS) -o serial_func.o serial_func.c
	cc -g -c $(CFLAGS) -o curses_func.o curses_func.c 
	cc -g $(CFLAGS) $(obj) -o main
clean:
	-rm -v main $(obj) errorfile
