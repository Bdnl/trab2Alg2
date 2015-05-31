CC=gcc
FLAGS=
CFLAGS=-c $(FLAGS)
LDFLAGS=$(FLAGS)
SOURCES=main.c misc.c registro.c database.c test.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=t2

all: $(SOURCES) $(EXECUTABLE)
	
run: $(EXECUTABLE)
	./$(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

debug: $(SOURCES)
	$(CC) $(SOURCES) -o $(EXECUTABLE) $(LDFLAGS) -DDEBUG
	./$(EXECUTABLE)

.c.o:
	$(CC) $< -o $@ $(CFLAGS)

clear:
	rm $(EXECUTABLE)
	rm *.dat
	rm *.o