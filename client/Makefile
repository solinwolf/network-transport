client:client.c
	gcc client.c net.c sqlite.c -o  client  -lssl -lcrypto -lsqlite3

.PHONY:clean
clean:
	rm -r client
