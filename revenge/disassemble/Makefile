prefix     = /usr/local

VERSION    = 0.0.1
PACKAGE    = dis

EXTRA_DIST = Makefile
INSTALL    = install -s -c
MAKE       = make

CC         = g++
SOURCES    = opcodes.cpp dis.cpp prefix_0f.cpp
OBJECTS    = opcodes.o dis.o prefix_0f.o
HEADERS    =

CFLAGS     = -Wall -O2 -pipe -DPACKAGE=\"$(PACKAGE)\" -DVERSION=\"$(VERSION)\" 
LDFLAGS    = 

.SUFFIXES: .cpp

.cpp:
	$(CC) ${CFLAGS} $<@

$(PACKAGE): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(PACKAGE) $(OBJECTS)

all: $(PACKAGE)

debug:
	$(MAKE) CFLAGS="-Wall -O -g -DDEBUG -DPACKAGE=\\\"$(PACKAGE)\\\" -DVERSION=\\\"$(VERSION)\\\""

install: all
	$(INSTALL) $(PACKAGE) $(prefix)/bin/$(PACKAGE)

dist: clean
	@(rm -rf $(PACKAGE)-$(VERSION) && mkdir $(PACKAGE)-$(VERSION) && \
	 cp -f $(EXTRA_DIST) $(SOURCES) $(HEADERS) $(PACKAGE)-$(VERSION) && \
	 tar -cjvf $(PACKAGE)-$(VERSION).tar.bz2 $(PACKAGE)-$(VERSION) && \
	 rm -rf $(PACKAGE)-$(VERSION))

clean:
	rm -f $(PACKAGE)*.tar.bz2
	rm -f *~ *.o $(PACKAGE)

