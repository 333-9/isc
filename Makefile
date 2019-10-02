CC = tcc

isc: isc.c parser/scanner.o y.tab.o terminal.c draw.o table.o
	$(CC) -o $@ isc.c terminal.c parser/scanner.o y.tab.o draw.o table.o

y.tab.o: y.tab.c
y.tab.c: parser/parser.y
	yacc -d parser/parser.y
parser/y.tab.h: y.tab.c
	ln -sr y.tab.h parser/

parser/scanner.o: parser/y.tab.h
