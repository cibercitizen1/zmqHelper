
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

  std::cerr << "client starts ... \n" << std::flush;

  SocketAdaptor< ZMQ_REQ > sa;

  sa.connect ("tcp://localhost:5555");

  //  Do 10 requests, waiting each time for a response
  int i;
  for (i = 1; i <= N; i++) {

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

  assert (i = N+1);

  std::cout << " OK \n" << std::flush;

  sa.close ();
  return 0;
} // main ()
