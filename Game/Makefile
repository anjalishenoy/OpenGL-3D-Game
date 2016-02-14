CC = g++
CFLAGS = -Wall
PROG = game

SRCS = main.cpp glad.c
LIBS = -ldl -lglfw -lGL 

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)