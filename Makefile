# this is makefile for MeMS
all: clean example 

example:
	gcc -o example example.c

clean:
	rm -rf example
