CC=gcc
FLAGS=
CFLAGS=-c $(FLAGS)
LDFLAGS=$(FLAGS)
SOURCES=main.c misc.c registro.c database.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=t2

all: $(SOURCES) $(EXECUTABLE)
	
run: $(EXECUTABLE)
	./$(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $< -o $@ $(CFLAGS)

clear:
	rm *.o
	rm "db.dat"
	rm "idx.dat"
	rm "idade.dat"
	rm "idade_list.dat"
	rm "generos.dat"
	rm "generos_list.dat"
	rm "generos_table.dat"
	rm $(EXECUTABLE)