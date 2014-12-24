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
void callback (zmq::socket_t & socket ) ;

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  SocketAdaptor< ZMQ_REP > sa; 

  sa.bind ("tcp://*:5555");

  //  would also work: sa.onMessage (callback);

  sa.onMessage (  [&]  (zmq::socket_t & socket ) -> void { 
	  
	  //  Get the request.
	  std::vector<std::string> lines = receiveText (socket);
	  
	  std::cout << " received -------- \n";
	  for ( auto s : lines ) {
		std::cout << s << "\n";
	  }
	  std::cout << " ----------------- \n";
	  
	  // Send the reply
	  std::vector<std::string> multi = { "Welt Welt", "World World" };
	  sendText ( socket, multi );
	  
	} );
  
  sa.wait (); // never returns because we don't stop the thread on 'onMessage()'

  return 0;
} // main ()

// ---------------------------------------------------------------
// ---------------------------------------------------------------
void callback (zmq::socket_t & socket ) {

  //  Get the request.
  auto lines = receiveText (socket);
  
  std::cout << " received -------- \n";
  for ( auto s : lines ) {
	std::cout << s << "\n";
  }
  std::cout << " ----------------- \n";

  // Send the reply
  std::vector<std::string> multi = { "Welt Welt", "World World" };
  sendText ( socket, multi );

}
