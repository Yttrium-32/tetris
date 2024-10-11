all: main run

main: ./src/main.c
	gcc -o main.o ./src/main.c `sdl2-config --cflags --libs`
run: main.o
	./main.o
clean:
	rm main.o
