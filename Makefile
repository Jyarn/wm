CC=gcc
CFLAGS=-g -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes
EXE=wm

OBJ=wm.o event.o config.o debug.o
INC=-I/usr/include
LIB=-lX11 -lXrandr

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LIB) $(OBJ) -o $(EXE)

config.o: config.c *.h
	$(CC) -g -Wall -Wextra $(INC) -c $< -o $@

%.o: %.c *.h
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

.PHONY: %.h

.PHONY: clean
clean:
	rm $(OBJ)
	rm $(EXE)
