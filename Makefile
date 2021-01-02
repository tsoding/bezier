CFLAGS_WITHOUT_PKGS=-Wall -Wextra -std=c11 -pedantic
PKGS=sdl2 epoxy

ifdef OS # windows nt
CFLAGS=$(CFLAGS_WITHOUT_PKGS) $(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -lm
# probably needs for 32b version : pacman -Sy mingw32/mingw-w64-i686-libepoxy
else # linux
CFLAGS=$(CFLAGS_WITHOUT_PKGS) `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm
# probably needs : sudo apt-get install libepoxy-dev
endif


all: cpu gpu

cpu: cpu.c
	$(CC) $(CFLAGS) -o cpu cpu.c $(LIBS)

gpu: gpu.c
	$(CC) $(CFLAGS) -o gpu gpu.c $(LIBS)
