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
int main () {


  //
  //
  //
  SocketAdaptorWithThread< ZMQ_REP > sa
  {	
	[] (SocketAdaptor<ZMQ_REP> &  socket ) -> void { 
	  
	  std::cerr << " *** user provided callback starting \n";
	  
	  std::vector<std::string> lines;
	  
	  // publish the service
	  socket.bind ("tcp://*:5555");
	  
	  while ( socket.receiveText (lines) ) {
		//  a request arrived
		
		std::cout << " received -------- \n";
		for ( auto s : lines ) {
		  std::cout << s << "\n";
		}
		std::cout << " ----------------- \n";
		
		// Send the reply
		std::vector<std::string> multi 
		  = { "Welt Welt", "World World" };
		socket.sendText ( multi );
	  } // while
	  
	}  // lambda
  }; // sa

  std::cerr << " main: before join the thread \n" << std::flush;
  
  sa.joinTheThread (); // never returns because we don't stop the thread on sa

  return 0;
} // main ()
