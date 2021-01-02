PKGS=sdl2 gl
CFLAGS=-Wall -Wextra -std=c11 -pedantic `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm

bezier: main.c
	$(CC) $(CFLAGS) -o bezier main.c $(LIBS)
