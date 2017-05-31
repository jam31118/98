CC = gcc 
CFLAGS = -O2 -Wall -lrt -pthread --std=c11
OUTPUT = problem3
OBJ = $(OUTPUT).c
#SUBDIRS = $(wildcard */.)

all: main

main: $(OBJ)
	$(CC) -o $(OUTPUT) $^ $(CFLAGS)

clean:
	$(RM) $(OUTPUT)

#clean-exe: 
#	rm -f $(OUTPUT)

#clean-outfile: $(SUBDIRS)
#	for subdir in $(SUBDIRS); do rm -f $${subdir}/*.o; done

