server:server.c
	gcc server.c net.c thread.c -o server -L/usr/lib -lssl -lcrypto -lpthread

.PHONY:clean
clean:
	rm -r server
