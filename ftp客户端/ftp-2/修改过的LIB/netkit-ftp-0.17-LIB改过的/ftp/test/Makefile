LIBS += -lpthread
test: ../cmds.o ../cmdtab.o ../domacro.o ../ftp.o ../glob.o ../libftp.o main.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

	
clean:
	rm *.o test