all:	bsd-ce nk-dump

bsd-ce:	bsd-ce.c
	$(CC) -o bsd-ce bsd-ce.c

nk-dump: nk-dump.c
	$(CC) -o nk-dump nk-dump.c

clean:
	rm -f bsd-ce nk-dump