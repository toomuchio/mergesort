#Compiler.
CC = gcc

#Compiler flags.
CFLAGS = -Wall -ansi -pedantic

#Finker flags.
LDFLAGS = -lm

all: sortmerge

#Link objects
sortmerge: libutil.o sortmerge.o
	$(CC) $(LDFLAGS) -o sortmerge libutil.o sortmerge.o

#Create objects
libutil.o: libutil.c
	$(CC) $(CFLAGS) -c libutil.c -o libutil.o

sortmerge.o: sortmerge.c
	$(CC) $(CFLAGS) -c sortmerge.c -o sortmerge.o

#Clean
clean:
	-@rm -rf *.o sortmerge core 2>/dev/null || true

