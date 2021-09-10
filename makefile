compile:
	gcc main.c func.c serial_func.c -lncurses -o main
valgr:debug
	rm aaa.txt
	touch aaa.txt
	valgrind $(VFLAGS) --track-origins=yes --leak-check=full --log-file=aaa.txt ./main
debug:
	gcc $(CFLAGS) main.c func.c serial_func.c -lncurses -g -o main
