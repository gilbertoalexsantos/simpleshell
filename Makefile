.SILENT: t comp

comp:
	gcc shell.c -o shell;

t: comp shell
	./shell
