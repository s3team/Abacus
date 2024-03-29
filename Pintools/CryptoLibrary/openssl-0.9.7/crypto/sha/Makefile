#
# OpenSSL/crypto/sha/Makefile
#

DIR=    sha
TOP=    ../..
CC=     cc
CPP=    $(CC) -E
INCLUDES=
CFLAG=-g
INSTALL_PREFIX=
OPENSSLDIR=     /usr/local/ssl
INSTALLTOP=/usr/local/ssl
MAKEDEPPROG=	makedepend
MAKEDEPEND=	$(TOP)/util/domd $(TOP) -MD $(MAKEDEPPROG)
MAKEFILE=       Makefile
AR=             ar r

SHA1_ASM_OBJ=

CFLAGS= $(INCLUDES) $(CFLAG)
ASFLAGS= $(INCLUDES) $(ASFLAG)

GENERAL=Makefile
TEST=shatest.c sha1test.c
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC=sha_dgst.c sha1dgst.c sha_one.c sha1_one.c
LIBOBJ=sha_dgst.o sha1dgst.o sha_one.o sha1_one.o $(SHA1_ASM_OBJ)

SRC= $(LIBSRC)

EXHEADER= sha.h
HEADER= sha_locl.h $(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:    lib

lib:    $(LIBOBJ)
	$(AR) $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB) || echo Never mind.
	@touch lib

# elf
asm/sx86-elf.s: asm/sha1-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) sha1-586.pl elf $(CFLAGS) $(PROCESSOR) > sx86-elf.s)

# a.out
asm/sx86-out.o: asm/sx86unix.cpp
	$(CPP) -DOUT asm/sx86unix.cpp | as -o asm/sx86-out.o

# bsdi
asm/sx86bsdi.o: asm/sx86unix.cpp
	$(CPP) -DBSDI asm/sx86unix.cpp | sed 's/ :/:/' | as -o asm/sx86bsdi.o

asm/sx86unix.cpp: asm/sha1-586.pl ../perlasm/x86asm.pl
	(cd asm; $(PERL) sha1-586.pl cpp $(PROCESSOR) >sx86unix.cpp)

asm/sha1-ia64.s:  asm/sha1-ia64.pl
	(cd asm; $(PERL) sha1-ia64.pl ../$@ $(CFLAGS))

files:
	$(PERL) $(TOP)/util/files.pl Makefile >> $(TOP)/MINFO

links:
	@$(PERL) $(TOP)/util/mklink.pl ../../include/openssl $(EXHEADER)
	@$(PERL) $(TOP)/util/mklink.pl ../../test $(TEST)
	@$(PERL) $(TOP)/util/mklink.pl ../../apps $(APPS)

install:
	@headerlist="$(EXHEADER)"; for i in $$headerlist ; \
	do  \
	(cp $$i $(INSTALL_PREFIX)$(INSTALLTOP)/include/openssl/$$i; \
	chmod 644 $(INSTALL_PREFIX)$(INSTALLTOP)/include/openssl/$$i ); \
	done;

tags:
	ctags $(SRC)

tests:

lint:
	lint -DLINT $(INCLUDES) $(SRC)>fluff

depend:
	$(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(PROGS) $(LIBSRC)

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f asm/sx86unix.cpp asm/*-elf.* *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff asm/*.o

# DO NOT DELETE THIS LINE -- make depend depends on it.

sha1_one.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
sha1_one.o: ../../include/openssl/opensslconf.h
sha1_one.o: ../../include/openssl/opensslv.h ../../include/openssl/safestack.h
sha1_one.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
sha1_one.o: ../../include/openssl/symhacks.h sha1_one.c
sha1dgst.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
sha1dgst.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
sha1dgst.o: ../../include/openssl/fips.h ../../include/openssl/lhash.h
sha1dgst.o: ../../include/openssl/opensslconf.h
sha1dgst.o: ../../include/openssl/opensslv.h ../../include/openssl/safestack.h
sha1dgst.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
sha1dgst.o: ../../include/openssl/symhacks.h ../md32_common.h sha1dgst.c
sha1dgst.o: sha_locl.h
sha_dgst.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
sha_dgst.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
sha_dgst.o: ../../include/openssl/fips.h ../../include/openssl/lhash.h
sha_dgst.o: ../../include/openssl/opensslconf.h
sha_dgst.o: ../../include/openssl/opensslv.h ../../include/openssl/safestack.h
sha_dgst.o: ../../include/openssl/sha.h ../../include/openssl/stack.h
sha_dgst.o: ../../include/openssl/symhacks.h ../md32_common.h sha_dgst.c
sha_dgst.o: sha_locl.h
sha_one.o: ../../include/openssl/crypto.h ../../include/openssl/e_os2.h
sha_one.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
sha_one.o: ../../include/openssl/safestack.h ../../include/openssl/sha.h
sha_one.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
sha_one.o: sha_one.c
