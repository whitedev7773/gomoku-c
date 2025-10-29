CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -lncurses
TARGET = gomoku

SRC = $(wildcard src/**/*.c) $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ -lncurses

clean:
	rm -f $(OBJ) $(TARGET)
