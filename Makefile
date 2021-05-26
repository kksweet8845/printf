CC = gcc
CFLAGS += -std=gnu99 -Wall -g -I./include


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main: printf.o
	make clean
	$(CC) $(CFLAGS) -o main main.c printf.o


clean:
	rm -f main