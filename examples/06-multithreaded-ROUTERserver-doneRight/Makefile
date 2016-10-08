include ../Makefile.in


all:
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) client.cpp -lzmq -o run.client
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) server.cpp -lzmq -o run.server 

clean:
	rm -f *.o run.*
