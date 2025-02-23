SHELL := /bin/bash
CC = gcc
TARGET = server
CFLAGS = -Wall -Wextra -ggdb -O0
OBJS = main.o
all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS)
%.o: src/%.c
	$(CC) $(CFLAGS) -c $<
clean:
	@rm -f $(OBJS) $(TARGET)
run:
	./$(TARGET)