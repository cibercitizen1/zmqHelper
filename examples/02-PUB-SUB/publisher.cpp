// ---------------------------------------------------------------
// publisher.cpp
// ---------------------------------------------------------------

// ---------------------------------------------------------------
// ---------------------------------------------------------------
#include <string>
#include <vector>
#include <stdlib.h>

#include "../../zmqHelper.hpp"

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  using namespace zmqHelper;

  const int N = 20;

  SocketAdaptor< ZMQ_PUB > sa; 
  
  sa.bind ("tcp://*:5555");
  
  int i;
  
  for (i=1; i<=N; i++) {
	std::cout << " publishing " << i << "\n";
	
	std::vector<std::string> multi = { "news", "nachrichten" };
	sa.sendText ( multi );
	
	sleep (1);
  } // true

  assert ( i = N+1);

  std::cout << " happy ending ! \n";
  
  return 0;
}

