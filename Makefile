CFLAG = -Wall -std=c99
all: main.o process.o scheduler.o
	gcc $(CFLAG) main.o process.o scheduler.o -o main
main.o: main.c
	gcc $(CFLAG) main.c -c
process.o: process.c
	gcc $(CFLAG) process.c -c
scheduler.o: scheduler.c
	gcc $(CFLAG) scheduler.c -c
clean:
	rm -f *.o
run:
	sudo ./main