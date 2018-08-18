PROG = lzma
CXX = gcc
LIB =
RM = rm -f
CFLAGS = -c -O2 -Wall -D_7ZIP_ST

OBJS = \
  main.o\
  LzmaUtil.o \
  Alloc.o \
  LzFind.o \
  LzmaDec.o \
  LzmaEnc.o \
  7zFile.o \
  7zStream.o \


all: $(PROG)

$(PROG): $(OBJS)
	$(CXX) -o $(PROG) $(LDFLAGS) $(OBJS) $(LIB) $(LIB2)
main.o: main.c
	$(CXX) $(CFLAGS) main.c

LzmaUtil.o: ../Lzma/LzmaUtil.c
	$(CXX) $(CFLAGS) ../Lzma/LzmaUtil.c

Alloc.o: ../Lzma/Alloc.c
	$(CXX) $(CFLAGS) ../Lzma/Alloc.c

LzFind.o: ../Lzma/LzFind.c
	$(CXX) $(CFLAGS) ../Lzma/LzFind.c

LzmaDec.o: ../Lzma/LzmaDec.c
	$(CXX) $(CFLAGS) ../Lzma/LzmaDec.c

LzmaEnc.o: ../Lzma/LzmaEnc.c
	$(CXX) $(CFLAGS) ../Lzma/LzmaEnc.c

7zFile.o: ../Lzma/7zFile.c
	$(CXX) $(CFLAGS) ../Lzma/7zFile.c

7zStream.o: ../Lzma/7zStream.c
	$(CXX) $(CFLAGS) ../Lzma/7zStream.c

clean:
	-$(RM) $(PROG) $(OBJS)
