CC=gcc
CFLAGS=-g -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -Wpedantic
EXE=wm

OBJ=wm.o util.o dbg.o
INC=-I/usr/include
LIB=-lX11 -lXrandr

all: $(EXE)

run: $(OBJ) test.o
	$(CC) $(LIB) $(OBJ) test.o -o test
	valgrind --leak-check=full --show-leak-kinds=all -s ./test

$(EXE): $(OBJ) main.o
	$(CC) $(LIB) $(OBJ) main.o -o $(EXE)

config.o: config.c *.h
	$(CC) -g -Wall -Wextra -Wpedantic $(INC) -c $< -o $@

%.o: %.c *.h
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

.PHONY: %.h

.PHONY: clean
clean:
	rm *.o
	rm $(EXE)
