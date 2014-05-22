.SILENT: t comp

comp:
	gcc shell.c -o shell.o;

t: comp shell.o
	./shell.o
