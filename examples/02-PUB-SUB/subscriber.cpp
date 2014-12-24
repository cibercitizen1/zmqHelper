// ---------------------------------------------------------------
// subscriber.cpp
// ---------------------------------------------------------------

#include <string>
#include <vector>
#include <stdlib.h>

#include "../../zmqHelper.h"

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  using namespace zmqHelper;

  SocketAdaptor< ZMQ_SUB > sa; 

  sa.connect ("tcp://localhost:5555");
  sa.subscribe ("news");

  while (true) {
	
	std::cout << " waiting -------- \n";

	//  Get the news
	auto lines = sa.receiveText ();
	  
	std::cout << " received -------- \n";
	for ( auto s : lines ) {
	  std::cout << s << "\n";
	}
	std::cout << " ----------------- \n";

  } // while true

  return 0;
}

