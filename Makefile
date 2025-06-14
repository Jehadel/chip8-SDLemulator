CFLAGS=-Wall -g
CC=gcc

all:
	$(CC) $(CFLAGS) chip8.c -Iinclude -Llib -lSDL2main -lSDL2 -o chip8
	./chip8

run:
	./chip8

comp:
	$(CC) $(CFLAGS) chip8.c -Iinclude -Llib -lSDL2main -lSDL2 -o chip8

clean:
	rm -f ./chip8