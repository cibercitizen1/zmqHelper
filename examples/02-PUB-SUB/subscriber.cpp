// ---------------------------------------------------------------
// subscriber.cpp
// ---------------------------------------------------------------

#include <string>
#include <vector>
#include <stdlib.h>

#include "../../zmqHelper.hpp"

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  using namespace zmqHelper;

  std::vector<std::string> lines;

  SocketAdaptor< ZMQ_SUB > sa; 

  sa.connect ("tcp://localhost:5555");
  sa.subscribe ("news");

  while ( sa.receiveText (lines) ) {
	
	//  there are news
	  
	std::cout << " news -------- \n";
	for ( auto s : lines ) {
	  std::cout << s << "\n";
	}
	std::cout << " ----------------- \n";

  } // while true

  return 0;
}

