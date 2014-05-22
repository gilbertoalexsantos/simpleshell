.SILENT: t comp

comp:
	gcc shell.c -o shell.o;

t: comp s.o
	./shell.o
