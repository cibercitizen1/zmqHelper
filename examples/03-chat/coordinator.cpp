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

  receiver.onMessage ( [&] (decltype(receiver) & socket) {
	  // the thread executing this callback is not
	  // the main-thread, but they are not
	  // contending for using the sockets
	  auto lines = socket.receiveText ();
	  
	  std::cout << " msg received= "; 
	  for ( auto s : lines ) {
		std::cout << s << " | ";
	  }
	  std::cout << "\n";

	  // publish
	  emitter.sendText (lines);

	  // answer to the guest
	  socket.sendText ("OK");
	 
	} );

  // never returns (receiver threds are never stopped)
  emitter.joinTheThread ();
  receiver.joinTheThread ();

} // () main
