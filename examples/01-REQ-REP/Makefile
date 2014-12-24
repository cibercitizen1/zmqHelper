include ../Makefile.in


all:
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) clientREQ.cpp -lzmq -o run.client
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) serverREP.cpp -lzmq -o run.server 

clean:
	rm -f *.o run.*
