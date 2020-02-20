all: myPipeLine


myPipeLine: myPipeLine.o
	gcc -g -m32 -Wall -o myPipeLine myPipeLine.o 


myPipeLine.o: myshell.c
	gcc -m32 -g -Wall -ansi -c -o myPipeLine.o myshell.c

clean:
	rm -f *o myPipeLine
