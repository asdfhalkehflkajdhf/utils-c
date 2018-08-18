TARGET=ftp
CSOURCES=ftp.c FtpIp.c FtpConnection.c regextest.c
INCLUDES=
LIBS=

CC=gcc
CXX=g++
CPPFLAGS=
CFLAGS=-O0 -g3 -Wall
CXXFLAGS=$(CFLAGS)
#LDFLAGS=-lefence
LDFLAGS=
LNK=$(CC)

OBJECTS=$(CSOURCES:.c=.o)
HEADERS=$(SOURCES:.c=.h)

%.o : %.cpp 
	$(CXX) -I. -I.. $(CXXFLAGS) $(INCLUDES) -I$(QTINCLUDEDIR) -c $< -o $@

%.o : %.c 
	$(CC) -I. -I.. $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY:all clean

all:$(TARGET)
	
clean:
	rm -f $(OBJECTS) $(TARGET)

$(TARGET):$(OBJECTS)
	$(LNK) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)
	
regex:unix.c regex.c
	$(CC) -I. -I.. $(CFLAGS) $(INCLUDES) -c parser.c -o parser.o
	$(CC) -I. -I.. $(CFLAGS) $(INCLUDES) -c regex.c -o regex.o
	$(LNK) -o regex parser.o regex.o $(LDFLAGS) $(LIBS)

