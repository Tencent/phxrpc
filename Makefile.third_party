
LIBS = libr3c.a

LIBR3C_OBJS = r3c/r3c.o r3c/crc16.o

all: $(LIBS)

libr3c.a: $(LIBR3C_OBJS)
	ar cru r3c/$@ $^

r3c/r3c.o: r3c/r3c.cpp
	gcc -c -g2 $< -I. -o $@ 

r3c/crc16.o: r3c/crc16.cpp
	gcc -c -g2 $< -I. -o $@ 

clean:
	rm -f $(LIBR3C_OBJS)
