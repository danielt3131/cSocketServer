SHELL := /bin/bash
CC = gcc
TARGET = server
CFLAGS = -Wall -Wextra -ggdb -O2 -fno-omit-frame-pointer -march=native -mtune=native
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
