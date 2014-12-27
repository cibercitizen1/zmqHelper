zmqHelper
=========

Helper class and functions on top of zmq C++ binding.

* Customize example/Makefile.in to your local settings (where your zmq library is).
(Remember                                                       export
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/pathToYour/zeromq/lib).


* The guide says: "If you're sharing sockets across threads, don't. It
will lead to random weirdness, and crashes."
  We have here one dedicated thread waiting for incoming data (onMessage()) which
	calls the user provided callback() to handle it.
 Thus, danger is limited to the case when the calback is reading data, and
 the main or a different thread sends data or is doing something else with the socket.
 (sendText() receiveText() are protected through a lock).

* Usage example (just examples/04-REQ-broker-REP/broker.cpp)


```cpp
#include <zmq.hpp>
#include <string>
#include <iostream>

#include "../../zmqHelper.h"

using namespace zmqHelper;

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
```

