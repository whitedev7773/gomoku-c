CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude -pthread
LDFLAGS = -lncurses -pthread
TARGET = gomoku

SRC := $(shell find src -name '*.c')
OBJ := $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
