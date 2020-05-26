CFLAGS := $(CFLAGS) -O3
all:
	cc dash.c -o dash -lSDL2 -lm $(CFLAGS)
