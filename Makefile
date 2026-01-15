CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -O3
PKG=`pkg-config --cflags --libs notcurses`

SRC=src/*
INC=-Iinclude -lm

tui: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(INC) $(PKG) -o tui

clean:
	rm -f tui
