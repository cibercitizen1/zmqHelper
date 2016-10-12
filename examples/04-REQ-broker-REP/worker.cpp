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

  socket.connect ("tcp://localhost:8001");

  std::vector<std::string> lines;

  const int N = 5;
  int count = 0;

  while (socket.receiveText (lines) ) {
  
	  std::cout << " *** worker received: -------- \n";
	  for ( auto s : lines ) { std::cout << s << "\n"; }
	  std::cout << " ----------------- \n";
	  
	  // Send the reply
	  std::vector<std::string> multi = { "Welt Welt", "World World" };
	  socket.sendText ( multi );

	  if ( ++count == N) break;
  } // while

  socket.close ();

  assert (count == N);

  std::cout << " *** worker thread happy ending \n";

}

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  // the worker only serves N (5) requests
  // so start many workers, or restar this programa
  // serveral time, not to leave clients unatended

  SocketAdaptorWithThread< ZMQ_REP > sa  { callback };

  std::cout << " *** worker main, going to join \n";
  
  sa.joinTheThread (); 

  std::cout << " *** worker main happy ending \n";

  return 0;
} // main ()

