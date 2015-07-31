all:
	gcc imgctool.c -o imgctool -lncurses -Wall -O0 -g

release:
	gcc imgctool.c -o imgctool -lncurses -Wall -O3
