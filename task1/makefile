all: myshell


myshell: myshell.o LineParser.o
	gcc -g -m32 -Wall -o myshell myshell.o LineParser.o


myshell.o: myshell.c
	gcc -m32 -g -Wall -ansi -c -o myshell.o myshell.c


LineParser.o: LineParser.c
	gcc -m32 -g -Wall -ansi -c -o LineParser.o LineParser.c


clean:
	rm -f *o myshell