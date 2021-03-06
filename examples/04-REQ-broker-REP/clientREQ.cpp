
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

  SocketAdaptor< ZMQ_REQ > sa;
  std::cerr << "Connecting to hello world server..." << std::endl;
  sa.connect ("tcp://localhost:8000");

  //  Do 10 requests, waiting each time for a response
  for (int i = 1; i <= 10; i++) {

	//  Send the request
	std::vector<std::string> multi = { "Hallo Hallo", "Hello Hello" };
	sa.sendText ( multi );

	std::cerr << " text sent, wating reply\n";

	//  Get the reply (ignore return)
	sa.receiveText (lines);
	  
	std::cout << " received -------- \n";
	for ( auto s : lines ) {
	  std::cout << s << "\n";
	}
	std::cout << " ----------------- \n";
	
  } // for
  return 0;
} // main ()
