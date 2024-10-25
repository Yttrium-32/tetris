all: main run
debug: main-debug run

main-debug:
	bear -- gcc -l SDL2_ttf -O0 -g -o main.o ./src/main.c `sdl2-config --cflags --libs`
main: ./src/main.c
	bear -- gcc -o main.o ./src/main.c `sdl2-config --cflags --libs` -l SDL2_ttf
run: main.o
	./main.o
clean:
	rm main.o
	rm compile_commands.json
	rm core.dmp
