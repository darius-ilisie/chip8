all: build run #DEFAULT
build:
	gcc -lSDL2 src/main.c src/include/chip8.c src/include/inter.c -o chip8

run:
	./chip8 Maze.ch8

clean:
	rm chip8
