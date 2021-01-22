CC = gcc 
CFLAGS = -m32
CFLAGS_DEBUG = -m32 -DDEBUG -g
LDFLAGS= 

PROGRAMS = aes1 sha1a

all: $(PROGRAMS)

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: all

$(PROGRAMS):
	$(CC) $@.c $(CFLAGS) -o $@  $(LDFLAGS)

clean:
	rm $(PROGRAMS) 2>/dev/null; true 


