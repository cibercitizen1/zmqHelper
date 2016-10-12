// ---------------------------------------------------------------
// coordinator.cpp
// ---------------------------------------------------------------

#include <zmq.hpp>
#include <string>
#include <iostream>

#include "../../zmqHelper.hpp"

// -----------------------------------------------------------------
// -----------------------------------------------------------------
int main ()
{

  using namespace zmqHelper;
  
  SocketAdaptor< ZMQ_PUB > emitter;
  SocketAdaptor< ZMQ_REP > receiver;
  
  receiver.bind ("tcp://*:8000");
  emitter.bind ("tcp://*:8001");

  std::vector<std::string> lines;

  bool receiving = true;

  while (receiving) {

	// read
	receiving = receiver.receiveText (lines);
	  
	std::cout << " msg received= |"; 
	for ( auto s : lines ) {
	  std::cout << s << "|";
	}
	std::cout << "\n";

	// publish
	emitter.sendText (lines);

	// answer to the guest
	receiver.sendText ({"OK"});

  } // while


} // () main
