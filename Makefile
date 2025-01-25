# base = common.c raytiles.c ascui.c  

# clean:
# 	rm -f *.o main
# build:
# 	clean
# demo_ms:
# 	gcc main.c minesweeper.c common.c raytiles.c -g -lc -lm -lraylib -I /usr/local/include -o demo_ms.o
# demo_ui:
# 	gcc ascui_test.c common.c raytiles.c ascui.c -g -lc -lm -lraylib -I /usr/local/include -o demo_ui.o

# mem: 
# 	valgrind --leak-check=full ./output

#gcc tl_test.c raytiles.c -g -lc -lm -lraylib -I /usr/local/include -o main

CC=gcc
CFLAGS= -Wall -g -lc -lm -lraylib -I /usr/local/include
SRC=src
OBJ=obj
SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

BINDIR=bin
BIN=$(BINDIR)/main

all:$(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	$(RM) -r $(BINDIR)/* $(OBJ)/*