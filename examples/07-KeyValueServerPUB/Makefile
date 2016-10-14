include ../Makefile.in


all:
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) -L. client.cpp -lzmq -ljson11 -o run.client
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) -L. server.cpp -lzmq -ljson11 -o run.server 

clean:
	rm -f *.o run.*
