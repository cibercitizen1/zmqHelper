
// ---------------------------------------------------------------
// client.cpp
// ---------------------------------------------------------------

#include <string>
#include <vector>
#include <stdlib.h>

#include "../../zmqHelper.hpp"

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main() {

  std::cout << "thread main = " << std::this_thread::get_id() << "\n";

  using namespace zmqHelper;

  std::cout << " client starting \n";

  SocketAdaptor< ZMQ_DEALER > sa; 
  std::cout << " socket created \n";

  sa.connect ("tcp://127.0.0.1:5580");
  std::cout << " socket connected \n";
  
  std::vector<std::string> multi;
  std::vector<std::string> lines;
  int i;

  const int N = 20;

  std::cout << " sending ... ";
  for (int i=1; i<=N; i++) {
	// warning: firs line is empty if using DEALER socket
	multi = { "", "this is a test", "{\"value\": 1234}" };
	sa.sendText ( multi );
  }

  std::cout << " done. Waiting for answers. \n";

  for (i=1; i<=N; i++) {

	if (! sa.receiveText (lines) ) break;
	  
	std::cout << " got answer : -------- \n";
	for ( auto s : lines ) {
	  std::cout << s << "\n";
	}
	std::cout << " ----------------- \n";
  }

  assert (i == N+1);

  std::cout << " happy ending \n";

  return 0;
} // main ()

