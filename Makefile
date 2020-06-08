CC = clang
#CC = tcc
ccflags = -fno-caret-diagnostics -fno-diagnostics-show-option -fno-diagnostics-color -O0

objects = parser.o draw.o table.o regex.o

isc: isc.c $(objects) terminal.c config.h
	$(CC) $(ccflags) -o $@ isc.c -ledit $(objects) terminal.c

.c.o:
	$(CC) $(ccflags) -c $<

#y.tab.o: y.tab.c
#y.tab.h: y.tab.c
#y.tab.c: parser.y
#	yacc -d parser.y
#scanner.o: y.tab.h


.PHONY: install clean

install: isc
	mkdir -p  /usr/local/bin/
	cp -f isc /usr/local/bin/
	chmod 755 /usr/local/bin/isc

clean:
	rm -f *.o y.tab.*
