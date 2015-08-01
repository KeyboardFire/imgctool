all:
	gcc imgctool.c saverestore.c saverestore.h ictdata.h -o imgctool -lncurses -Wall -O0 -g

release:
	gcc imgctool.c saverestore.c saverestore.h ictdata.h -o imgctool -lncurses -Wall -O3
