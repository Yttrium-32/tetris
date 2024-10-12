all: main run
debug: main-debug run

main-debug:
	gcc -O0 -g -o main.o ./src/main.c `sdl2-config --cflags --libs`
main: ./src/main.c
	gcc -o main.o ./src/main.c `sdl2-config --cflags --libs`
run: main.o
	./main.o
clean:
	rm main.o
	rm core.dmp
