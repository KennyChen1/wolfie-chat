CFLAGS = -Wall -Werror -pthread -lpthread -lssl -lcrypto -g

all: server client chat sfwrite

server: server.c
	gcc server.c $(CFLAGS) -o $@

client: client.c
	gcc $(CFLAGS) client.c -o $@

chat: chat.c
	gcc $(CFLAGS) chat.c -o $@

sfwrite: sfwrite.c
	gcc sfwrite.c $(CFLAGS) -c 

clean:
	rm -f *~ *.o server client chat sfwrite