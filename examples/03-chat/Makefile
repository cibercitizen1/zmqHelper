include ../Makefile.in


all:
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) coordinator.cpp -lzmq -o run.coord
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) guest.cpp -lzmq -o run.guest
	$(CC) $(INCLUDE_DIRS) $(LIB_DIRS) guestBetter.cpp -lzmq -o run.guestBetter

clean:
	rm -f *.o run.*
