INCLUDE = -I. 
LIBDIR  = -L.

COMPILERFLAGS = -Wall -O2 -DSDL `sdl-config --cflags` 
CC = gcc
CXX = g++
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lm -lpthread `sdl-config --libs`
OBJS = targa.o unigraf.o estex.o z80/z80.o z80/z80_ops.o

all: sprint

z80/z80.o: z80/z80.cpp z80/z80.h
	$(CC) $(CFLAGS) -c z80/z80.cpp -o z80/z80.o

z80/z80_ops.o: z80/z80_ops.cpp z80/z80.h z80/z_cb.hpp z80/z_ddfd.hpp z80/z_ddfdcb.hpp z80/z_ed.hpp z80/z_macros.h
	$(CC) $(CFLAGS) -c z80/z80_ops.cpp -o z80/z80_ops.o

estex.o: estex.cpp estex.h sprint.h
	$(CC) $(CFLAGS) -c estex.cpp

unigraf.o: unigraf.cpp unigraf.h
	$(CC) $(CFLAGS) -c unigraf.cpp -DFONT8X8 -DUNIFOPEN

targa.o: targa.cpp targa.h
	$(CC) $(CFLAGS) -c targa.cpp

sprint: sprint.cpp estex.h sprint.h z80/z80.h $(OBJS)
	$(CXX) sprint.cpp $(CFLAGS) -o sprint $(LIBDIR) $(LIBRARIES) $(OBJS)


clean:
	$(RM) -f $(OBJS) sprint
