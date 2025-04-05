SHELL := /bin/bash
CC = gcc
TARGET = server
CFLAGS = -Wall -Wextra -ggdb -O0 -fsanitize=address
OBJS = main.o server.o
all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS) -lpthread -fsanitize=address -static-libasan
%.o: src/%.c
	$(CC) $(CFLAGS) -c $<
clean:
	@rm -f $(OBJS) $(TARGET)
run:
	./$(TARGET)