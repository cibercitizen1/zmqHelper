
// -----------------------------------------------------------------
// clientREQ.cpp
// -----------------------------------------------------------------

#include <zmq.hpp>
#include <string>
#include <iostream>

#include "../../zmqHelper.hpp"

// -----------------------------------------------------------------
// -----------------------------------------------------------------
int main ()
{
  using namespace zmqHelper;

  std::vector<std::string> lines;
  const int N = 10;

  SocketAdaptor< ZMQ_REQ > sa;
  std::cerr << "Connecting to hello world server..." << std::endl;
  sa.connect ("tcp://localhost:5555");

  //  Do 10 requests, waiting each time for a response
  for (int i = 1; i <= N; i++) {

	std::cout << " i = " << i << "\n";

	//  Send the request
	std::vector<std::string> multi = { "Hallo Hallo", "Hello Hello" };
	sa.sendText ( multi );

	//  Get the reply.
	sa.receiveText (lines); // ignoring bool returned. Guess it's true
	  
	std::cout << " received -------- \n";
	for ( auto s : lines ) {
	  std::cout << s << "\n";
	}
	std::cout << " ----------------- \n";
	
  } // for

  std::cout << " OK \n" << std::flush;

  sa.close ();
  return 0;
} // main ()
