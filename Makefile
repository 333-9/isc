CC = tcc

isc: isc.c scanner.o y.tab.o terminal.c draw.o table.o range.o config.h
	$(CC) -o $@ isc.c terminal.c scanner.o y.tab.o draw.o table.o range.o

y.tab.o: y.tab.c
y.tab.h: y.tab.c
y.tab.c: parser.y
	yacc -d parser.y
scanner.o: y.tab.h


.PHONY: install clean

install: isc
	mkdir -p /usr/local/bin/
	cp -f isc /usr/local/bin/
	chmod 755 /usr/local/bin/isc

clean:
	rm -fv *.o y.tab.*
