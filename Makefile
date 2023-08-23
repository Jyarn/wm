CC=gcc
CFLAGS=-g -Wall
FLAGS=
EXE=wm

OBJ=wm.o event.o
INC=-I/usr/include
LIB=-lX11

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LIB) $(OBJ) -o $(EXE)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

.PHONY: clean
clean:
	rm $(OBJ)
	rm $(EXE)