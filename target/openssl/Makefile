CC = gcc
CFLAGS = -m32 -lcrypto -DDEBUG

all: aes_128_cbc aes_256_cbc des_128_cbc md4 md5 sha1 sha256 sha384 sha512 rsa_key rc4 bigadd 

aes_128_cbc:
	$(CC) aes_128_cbc.c $(CFLAGS) -o aes_128_cbc

aes_256_cbc:
	$(CC) aes_256_cbc.c $(CFLAGS) -o aes_256_cbc

des_128_cbc:
	$(CC) des_ecb.c $(CFLAGS) -o des_128_ecb

md5:
	$(CC) md5.c $(CFLAGS) -o md5

md4:
	$(CC) md4.c $(CFLAGS) -o md4

sha1:
	$(CC) sha1.c $(CFLAGS) -o sha1

sha256:
	$(CC) sha256.c $(CFLAGS) -o sha256

sha384:
	$(CC) sha384.c $(CFLAGS) -o sha384

sha512:
	$(CC) sha512.c $(CFLAGS) -o sha512

rsa_key:
	$(CC) rsa_generate_key.c $(CFLAGS) -o rsa_key

rc4:
	$(CC) rc4.c $(CFLAGS) -o rc4

bigadd:
	$(CC) bigadd.c $(CFLAGS) -o bigadd

clean:
	rm aes_128_cbc aes_256_cbc des_128_ecb md5 sha1 sha256 sha384 sha512 rsa_key rc4 md4 bigadd
