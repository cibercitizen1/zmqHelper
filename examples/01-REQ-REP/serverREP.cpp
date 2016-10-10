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

  SocketAdaptor< ZMQ_REP > sa ( 

							   //  would also work: sa.onMessage (callback);
							   
							   [&]  (decltype(sa) & socket ) -> void { 

								 std::cerr << " *** user provided callback starting \n";
								 
								 socket.bind ("tcp://*:5555");
								 
								 while (true) {
								   
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
								 } // true
								 
							   } 
								); // sa

  // sa.bind ("tcp://*:5555"); now, main-thread can't do this
  
  sa.joinTheThread (); // never returns because we don't stop the thread on 'onMessage()'

  return 0;
} // main ()

// ---------------------------------------------------------------
// ---------------------------------------------------------------
void callback (SocketAdaptor<ZMQ_REP> & socket) {


}
