CC = tcc

isc: isc.c parser/scanner.o y.tab.o terminal.c draw.o table.o parser/range.o
	$(CC) -o $@ isc.c terminal.c parser/scanner.o y.tab.o draw.o table.o parser/range.o

y.tab.o: y.tab.c
y.tab.h: y.tab.c
y.tab.c: parser/parser.y
	yacc -d parser/parser.y
parser/y.tab.h: y.tab.h
	ln -srf y.tab.h parser/

parser/scanner.o: parser/y.tab.h

.PHONY: install

install: isc
	mkdir -p /usr/local/bin/
	cp -f isc /usr/local/bin/
	chmod 755 /usr/local/bin/isc
