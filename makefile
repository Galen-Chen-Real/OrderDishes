all:server client

CC=gcc
CFLAGES=-lpthread -lmysqlclient -g

server:server.c pipe.o thread_pool.o sock.o sql.o
	$(CC) $(CFLAGES) -o $@ $+
client:client.c pipe.o thread_pool.o sock.o
	$(CC) $(CFLAGES) -o $@ $+
.c.o:
	$(CC) $(CFLAGES) -c -o $*.o $<

clean:
	rm *.o server client
