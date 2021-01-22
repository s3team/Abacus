CC = gcc 
CFLAGS = -m32
CFLAGS_DEBUG = -m32 -DDEBUG -g
LDFLAGS= 

PROGRAMS = aes2 sha1b sha256a sha512a rc4a rc4b aes3 md5a sha256b sha512b md5b sha1c des1 des2

all: $(PROGRAMS)

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: all

$(PROGRAMS):
	$(CC) $@.c $(CFLAGS) -o $@  $(LDFLAGS)

clean:
	rm $(PROGRAMS) 2>/dev/null; true 


