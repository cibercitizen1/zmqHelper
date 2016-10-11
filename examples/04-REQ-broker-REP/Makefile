include ../Makefile.in


all:
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) clientREQ.cpp -lzmq -o run.client
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) worker.cpp -lzmq -o run.worker
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) broker.cpp -lzmq -o run.broker

clean:
	rm -f *.o run.*
