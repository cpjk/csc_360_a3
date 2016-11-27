CC = gcc

all: diskinfo

diskinfo: diskinfo.c
	$(CC) diskinfo.c utils.c -o diskinfo.o -g

disklist:
	$(CC) disklist.c utils.c -o disklist.o -g

diskget:
	$(CC) diskget.c utils.c -o diskget.o -g

run: all
	./diskinfo disk2.IMA

clean:
	rm *.o *.PDF
