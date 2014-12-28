// ---------------------------------------------------------------
// broker.cpp
// ---------------------------------------------------------------

#include <zmq.hpp>
#include <string>
#include <iostream>

#include "../../zmqHelper.h"

using namespace zmqHelper;

/*
 * ZeroMQ applications always start by creating a context, 
 * and then using that for creating sockets. 
 * In C, it's the zmq_ctx_new() call. 
 * You should create and use exactly one context in your process. 
 * Technically, the context is the container for all sockets 
 * in a single process, and acts as the transport for inproc sockets, 
 * which are the fastest way to connect threads in one process. 
 * If at runtime a process has two contexts, 
 * these are like separate ZeroMQ instances. 
 * If that's explicitly what you want, OK, but otherwise remember:
 * 
 * Do one zmq_ctx_new() at the start of your main line code, 
 * and one zmq_ctx_destroy() at the end.
 */

// -----------------------------------------------------------------
// -----------------------------------------------------------------
int main ()
{

  zmq::context_t theContext {1};
  // but also works each socket having its own context
  SocketAdaptor< ZMQ_ROUTER > frontend_ROUTER {theContext};
  SocketAdaptor< ZMQ_DEALER > backend_DEALER {theContext};

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
