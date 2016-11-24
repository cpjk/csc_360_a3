CC = gcc

all: diskinfo

diskinfo: diskinfo.c
	$(CC) diskinfo.c -o diskinfo

run: all
	./diskinfo disk2.IMA

clean:
	rm diskinfo
