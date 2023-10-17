CC=gcc
CFLAGS=-g -Wall -Wextra -Wstrict-prototypes
FLAGS=-D__DEBUG__
EXE=wm

OBJ=wm.o event.o config.o debug.o
INC=-I/usr/include
LIB=-lX11

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LIB) $(OBJ) -o $(EXE)

%.o: %.c *.h
	$(CC) $(CFLAGS) $(FLAGS) $(INC) -c $< -o $@

.PHONY: %.h

.PHONY: clean
clean:
	rm $(OBJ)
	rm $(EXE)