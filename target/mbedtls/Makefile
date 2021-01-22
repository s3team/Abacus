CC = gcc 
CFLAGS = -m32
CFLAGS_DEBUG = -m32 -DDEBUG
PWD= $(shell pwd)
MBEDTLS = $(PWD)/mbedtls/build/library
LDFLAGS= -L./mbedtls/build/library -Wl,-rpath $(MBEDTLS) -I./mbedtls/include -l:libmbedcrypto.a


# Note:  We're using rpath to tell the binary where to find mbedtls. 
#        Another way to do it is to run 
#            export LD_LIBRARY_PATH=`pwd`/mbedtls/build/library
#        in the shell before running the program

PROGRAMS = md5 md4 sha1 sha256 sha384 sha512 aes_128_cbc aes_256_cbc bigmul bigadd rc4 des_ecb

all: $(PROGRAMS)

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: all

$(PROGRAMS):
	$(CC) $@.c $(CFLAGS) -o $@  $(LDFLAGS)

clean:
	rm $(PROGRAMS) 2>/dev/null; true 
