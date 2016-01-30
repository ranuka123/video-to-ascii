all: vid.c test.c vid.h 
	gcc vid.c test.c -Wall -I/usr/local/include -L/usr/local/lib -lswscale -lavutil -lavcodec -lavformat -lz -lm -lavutil -I/usr/local/opt/ncurses/include -L/usr/local/opt/ncurses/lib -lncurses -o test
