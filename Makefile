CFLAGS=-Wall -Wextra -std=c11 -pedantic `pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2`

bezier: main.c
	$(CC) $(CFLAGS) -o bezier main.c $(LIBS)
