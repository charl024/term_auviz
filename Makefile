CC = gcc
CFLAGS = -Wall -Wextra -O3 
PKG = $(shell pkg-config --cflags --libs notcurses libpipewire-0.3)

SRC = src/*
INC = -Iinclude -lm -lpthread

TARGET = term_auviz

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(INC) $(PKG) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
