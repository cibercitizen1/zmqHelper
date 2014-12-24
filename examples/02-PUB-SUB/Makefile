include ../Makefile.in


all:
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) subscriber.cpp -lzmq -o run.subs
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) publisher.cpp -lzmq -o run.pub 

clean:
	rm -f *.o run.*
