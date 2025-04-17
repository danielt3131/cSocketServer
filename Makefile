SHELL := /bin/bash
CC = gcc
TARGET = server
CFLAGS = -Wall -Wextra -ggdb -O0 -fno-omit-frame-pointer
OBJS = main.o server.o threadpool.o
all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS) -lpthread
%.o: src/%.c
	$(CC) $(CFLAGS) -c $<
clean:
	@rm -f $(OBJS) $(TARGET)
run:
	./$(TARGET)
