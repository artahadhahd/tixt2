# TODO: make a better makefile
all:
	g++ *.cpp -I. -lncurses -Wall -Wextra -Os -o tixt

install:
	cp ./tixt /usr/bin