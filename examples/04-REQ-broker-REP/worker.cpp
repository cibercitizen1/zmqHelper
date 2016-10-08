// ---------------------------------------------------------------
// worker.cpp
// ---------------------------------------------------------------

#include <string>
#include <vector>
#include <stdlib.h>

#include "../../zmqHelper.hpp"

using namespace zmqHelper;

// ---------------------------------------------------------------
// ---------------------------------------------------------------
void callback (SocketAdaptor<ZMQ_REP> & socket ) {

  // The main-thread created the socket,
  // and it is different from the thread
  // using it, but they don't contend
  // so I guess this is safe.

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

