all : pseudu-shell


pseudu-shell: main.o command.o
	gcc -o pseudu-shell main.o command.o


main.o: main.c
	gcc -c main.c

command.o: command.c command.h
	gcc -c command.c


clean:
	rm -f core *.o pseudu-shell
