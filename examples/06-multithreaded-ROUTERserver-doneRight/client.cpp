
// ---------------------------------------------------------------
// client.cpp
// ---------------------------------------------------------------

#include <string>
#include <vector>
#include <stdlib.h>

#include "../../zmqHelper.hpp"

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main() {

  using namespace zmqHelper;

  SocketAdaptor< ZMQ_DEALER > sa; 
  sa.connect ("tcp://localhost:5580");

  
  std::vector<std::string> multi;
  std::vector<std::string> lines;

  const int N = 20;

  std::cout << " sending ... ";
  for (int i=1; i<=N; i++) {
	// warning: firs line is empty if using DEALER socket
	multi = { "", "this is a test", "{\"value\": 1234}" };
	sa.sendText ( multi );
  }

  std::cout << " done. Waiting for answers. \n";

  for (int i=1; i<=N; i++) {

	lines = sa.receiveText ();
	  
	std::cout << " got answer : -------- \n";
	for ( auto s : lines ) {
	  std::cout << s << "\n";
	}
	std::cout << " ----------------- \n";
  }

  return 0;
} // main ()

