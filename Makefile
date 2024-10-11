all: main run

main: ./src/main.c
	cc ./src/main.c -o main.o
run: main.o
	./main.o
clean:
	rm main.o
