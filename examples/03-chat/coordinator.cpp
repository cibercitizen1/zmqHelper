// ---------------------------------------------------------------
// coordinator.cpp
// ---------------------------------------------------------------

#include <zmq.hpp>
#include <string>
#include <iostream>

#include "../../zmqHelper.h"

// -----------------------------------------------------------------
// -----------------------------------------------------------------
int main ()
{

  using namespace zmqHelper;
  
  SocketAdaptor< ZMQ_PUB > emitter;
  SocketAdaptor< ZMQ_REP > receiver;
  
  receiver.bind ("tcp://*:8000");
  emitter.bind ("tcp://*:8001");

  auto & socketemitter = emitter.getSocket ();

  receiver.onMessage ( [&] (zmq::socket_t & socket ) {
	  auto lines = receiveText (socket);
	  
	  std::cout << " msg received= "; 
	  for ( auto s : lines ) {
		std::cout << s << " | ";
	  }
	  std::cout << "\n";

	  // publish
	  sendText (socketemitter, lines);

	  // answer to the guest
	  sendText (socket, "OK");
	 
	} );

  // never returns (receiver threds are never stopped)
  emitter.wait ();
  receiver.wait ();

} // () main
