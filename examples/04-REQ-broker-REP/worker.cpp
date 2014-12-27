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
void callback (SocketAdaptor<ZMQ_REP> & socket ) {

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

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  SocketAdaptor< ZMQ_REP > sa; 

  sa.connect ("tcp://localhost:8001");

  sa.onMessage (callback);
  
  sa.wait (); // never returns

  return 0;
} // main ()

