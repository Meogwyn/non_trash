CFLAGS = -lncurses -Wall -Os
obj = main.o func.o curses_func.o serial_func.o

.PHONY: clean mainclean ass

main:ass $(obj)
	cc $(CFLAGS) $(obj) -o main
mainclean:main
	-rm -v $(obj) #remove useless object files after compiling

main.o: curses_func.h serial_func.h
func.o:
curses_func.o:curses_func.h
serial_func.o:serial_func.h

valgr:debug_comp
	-rm -vf valgr_log.txt
	touch valgr_log.txt
	valgrind --track-origins=yes --leak-check=full --log-file=valgr_log.txt ./main

debug_comp:clean
	cc -g -c $(CFLAGS) -o main.o main.c 
	cc -g -c $(CFLAGS) -o func.o func.c 
	cc -g -c $(CFLAGS) -o serial_func.o serial_func.c
	cc -g -c $(CFLAGS) -o curses_func.o curses_func.c 
	cc -g $(CFLAGS) $(obj) -o main
clean:
	-rm -v main $(obj) errorfile valgr*
ass: 
	echo -e "\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
