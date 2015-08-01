FILES = $(wildcard src/*.c src/*.h)

all:
	gcc $(FILES) -o imgctool -lncurses -Wall -O0 -g

release:
	gcc $(FILES) -o imgctool -lncurses -Wall -O3
