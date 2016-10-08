// ---------------------------------------------------------------
// publisher.cpp
// ---------------------------------------------------------------

// ---------------------------------------------------------------
// ---------------------------------------------------------------
#include <string>
#include <vector>
#include <stdlib.h>

#include "../../zmqHelper.hpp"

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  using namespace zmqHelper;

  SocketAdaptor< ZMQ_PUB > sa; 
  
  sa.bind ("tcp://*:5555");
  
  int i=1;
  
  while (true) {
	std::cout << " publishing " << i << "\n";
	
	std::vector<std::string> multi = { "news", "nachrichten" };
	sa.sendText ( multi );
	
	sleep (1);
	i++;
  } // true
  
  return 0;
}

