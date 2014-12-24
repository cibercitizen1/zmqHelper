// ---------------------------------------------------------------
// worker.cpp
// ---------------------------------------------------------------

#include <string>
#include <vector>
#include <stdlib.h>

#include "../../zmqHelper.h"

using namespace zmqHelper;

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

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  SocketAdaptor< ZMQ_REP > sa; 

  sa.connect ("tcp://localhost:8001");

  sa.onMessage (callback);
  
  sa.wait (); // never happens

  return 0;
} // main ()

