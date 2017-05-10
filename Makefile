# the compiler to use
CC = clang
CFLAGS = -Wall -std=gnu99

# files to link:
LFLAGS = -lpthread -lm

INCL = -I includes

# the name to use for the target source file and the output file
TARGET = main
OBJS = solver

SEQTARGET = sequential
SEQOBJ = sequential

GENTARGET = generator
GENOBJ = generator

all: $(TARGET)

$(TARGET): src/$(TARGET).c
	$(CC) src/main.c src/solver.c -o $(OBJS) $(INCL) $(CFLAGS) $(LFLAGS)


sequential: src/$(SEQTARGET).c

$(SEQTARGET): src/$(SEQTARGET).c
	$(CC) src/sequential.c -o $(SEQOBJ) $(INCL) $(CFLAGS) $(LFLAGS)

generator: src/$(GENTARGET).c
	$(CC) src/generator.c -o $(GENOBJ) $(INCL) $(CFLAGS) $(LFLAGS)

clean:
	rm $(OBJS) $(SEQOBJ) $(GENOBJ)

