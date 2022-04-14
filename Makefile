all:
	gcc src/main.c src/include/chip8.c -o chip8
	./chip8 Maze.ch8
