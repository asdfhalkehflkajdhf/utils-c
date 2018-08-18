all: ftp libftp.so libftp.a

include ../MCONFIG
include ../MRULES

ifeq ($(USE_READLINE),1)
CFLAGS += -D__USE_READLINE__
LIBS += -lreadline $(LIBTERMCAP) 
endif
LIBS += -lpthread

ftp: cmds.o cmdtab.o domacro.o ftp.o glob.o libftp.o main.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@
libftp.so: cmds.c cmdtab.c domacro.c ftp.c glob.c libftp.c 
	$(CC) $^ -shared -fpic -o libftp.so
libftp.a:
	ar cr libftp.a cmds.o cmdtab.o domacro.o ftp.o glob.o libftp.o 

domacro.o ftp.o glob.o libftp.o : ftp_var.h pathnames.h
cmds.o cmdtab.o: ftp_var.h pathnames.h cmds.h
cmds.o glob.o: glob.h

install: ftp
	install -s -m$(BINMODE) ftp $(INSTALLROOT)$(BINDIR)
	ln -sf ftp $(INSTALLROOT)$(BINDIR)/pftp
	install -m$(MANMODE) ftp.1 $(INSTALLROOT)$(MANDIR)/man1
	ln -sf ftp.1 $(INSTALLROOT)$(MANDIR)/man1/pftp.1
	install -m$(MANMODE) netrc.5 $(INSTALLROOT)$(MANDIR)/man5

clean:
	rm -f *.o ftp *.so

