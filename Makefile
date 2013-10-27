CC=gcc

test: test.c http.c strlcpy.o
	$(CC) test.c http.c strlcpy.o -o test

server_f: server_f.c server_f.h http.o strlcpy.o io.o
	$(CC) server_f.h server_f.c http.o strlcpy.o -o server_f

http.o: http.c http.h

io.o: io.c io.h

strlcpy.o: strlcpy.c

clean:
	rm -f server_f *.o test
