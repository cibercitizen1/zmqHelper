// ---------------------------------------------------------------
// serverREP.cpp
// ---------------------------------------------------------------

#include <string>
#include <vector>
#include <stdlib.h>

#include "../../zmqHelper.hpp"

using namespace zmqHelper;

/*
* ON MULTITHREADING WITH ZeroMQ
*
* Remember: 
*
*       Do not use or close sockets except in the thread that created them.
*
*       Don't share ZeroMQ sockets between threads. 
*       ZeroMQ sockets are not threadsafe. 
* 
*       Isolate data privately within its thread and never share data 
*       in multiple threads. The only exception to this are ZeroMQ contexts, 
*       which are threadsafe.
*/


// ---------------------------------------------------------------
// ---------------------------------------------------------------
void callback (SocketAdaptor<ZMQ_REP> & socket) ;

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  SocketAdaptor< ZMQ_REP > sa; 

  sa.bind ("tcp://*:5555");

  //  would also work: sa.onMessage (callback);

  sa.onMessage (  [&]  (decltype(sa) & socket ) -> void { 

	  //
	  // CAUTION: Thread-main created socket sa (used here)
	  // but this is run by a different thread.
	  // Anyway, thread-main will be stopped on sa.wait()
	  // for ever, not contending for the socket.
	  //
	  
	  //  Get the request.
	  auto lines = socket.receiveText ();
	  
	  std::cout << " received -------- \n";
	  for ( auto s : lines ) {
		std::cout << s << "\n";
	  }
	  std::cout << " ----------------- \n";
	  
	  // Send the reply
	  std::vector<std::string> multi = { "Welt Welt", "World World" };
	  socket.sendText ( multi );
	  
	} );
  
  sa.wait (); // never returns because we don't stop the thread on 'onMessage()'

  return 0;
} // main ()

// ---------------------------------------------------------------
// ---------------------------------------------------------------
void callback (SocketAdaptor<ZMQ_REP> & socket) {

  //
  // CAUTION: Thread-main created socket sa (used here)
  // but this is run by a different thread.
  // Anyway, thread-main will be stopped on sa.wait()
  // for ever, not contending for the socket.
  //

  //  Get the request.
  auto lines = socket.receiveText ();
  
  std::cout << " received -------- \n";
  for ( auto s : lines ) {
	std::cout << s << "\n";
  }
  std::cout << " ----------------- \n";

  // Send the reply
  std::vector<std::string> multi = { "Welt Welt", "World World" };
  socket.sendText ( multi );

}
