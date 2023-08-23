CC=gcc
CFLAGS=-g -Wall
FLAGS=-D__DEBUG__
EXE=wm

OBJ=wm.o event.o config.o
INC=-I/usr/include
LIB=-lX11

all: $(EXE)

$(EXE): $(OBJ) config.h
	$(CC) $(LIB) $(OBJ) -o $(EXE)

%.o: %.c
	$(CC) $(CFLAGS) $(FLAGS) $(INC) -c $< -o $@

.PHONY: clean
clean:
	rm $(OBJ)
	rm $(EXE)