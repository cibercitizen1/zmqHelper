// ---------------------------------------------------------------
// serverREP.cpp
// ---------------------------------------------------------------

#include <string>
#include <vector>
#include <stdlib.h>

#include "../../zmqHelper.h"

using namespace zmqHelper;

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
