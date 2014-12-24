
// -----------------------------------------------------------------
// clientREQ.cpp
// -----------------------------------------------------------------

#include <zmq.hpp>
#include <string>
#include <iostream>

#include "../../zmqHelper.h"


// -----------------------------------------------------------------
// -----------------------------------------------------------------
int main ()
{
  using namespace zmqHelper;

  SocketAdaptor< ZMQ_REQ > sa;
  std::cerr << "Connecting to hello world server..." << std::endl;
  sa.connect ("tcp://localhost:5555");

  //  Do 10 requests, waiting each time for a response
  for (int i = 1; i <= 10; i++) {

	//  Send the request
	std::vector<std::string> multi = { "Hallo Hallo", "Hello Hello" };
	sa.sendText ( multi );

	//  Get the reply.
	auto lines = sa.receiveText ();
	  
	std::cout << " received -------- \n";
	for ( auto s : lines ) {
	  std::cout << s << "\n";
	}
	std::cout << " ----------------- \n";
	
  } // for
  return 0;
} // main ()
