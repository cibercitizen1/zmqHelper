// ---------------------------------------------------------------
// broker.cpp
// ---------------------------------------------------------------

#include <zmq.hpp>
#include <string>
#include <iostream>

#include "../../zmqHelper.h"

using namespace zmqHelper;

// -----------------------------------------------------------------
// -----------------------------------------------------------------
int main ()
{

  SocketAdaptor< ZMQ_ROUTER > frontend_ROUTER;
  SocketAdaptor< ZMQ_DEALER > backend_DEALER;

  frontend_ROUTER.bind ("tcp://*:8000");
  backend_DEALER.bind ("tcp://*:8001");

  // 
  frontend_ROUTER.onMessage ( [&] (SocketAdaptor<ZMQ_ROUTER> & socket ) {
	  auto lines = socket.receiveText ();
	  
	  std::cout << " msg received on FRONTEND = "; 
	  for ( auto s : lines ) { std::cout << s << " | "; }
	  std::cout << "\n";

	  // routing
	  backend_DEALER.sendText (lines);
	 
	} );

  // 
  backend_DEALER.onMessage ( [&] (SocketAdaptor<ZMQ_DEALER> & socket ) {
	  auto lines = backend_DEALER.receiveText ();
	  
	  std::cout << " msg received on BACKEND = "; 
	  for ( auto s : lines ) { std::cout << s << " | "; }
	  std::cout << "\n";

	  // routing
	  frontend_ROUTER.sendText (lines);
	 
	} );

  // never happens because we don't stop the receivers
  frontend_ROUTER.wait ();
  backend_DEALER.wait ();

} // () main
