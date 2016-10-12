// ---------------------------------------------------------------
// subscriber.cpp
// ---------------------------------------------------------------

#include <string>
#include <vector>
#include <stdlib.h>

#include "../../zmqHelper.hpp"

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  using namespace zmqHelper;

  const int N = 10;

  std::vector<std::string> lines;

  SocketAdaptor< ZMQ_SUB > sa; 

  sa.connect ("tcp://localhost:5555");
  sa.subscribe ("news");

  int i;
  for (i = 1; i<=N; i++) {

	if  ( ! sa.receiveText (lines) )  break;
	
	//  there are news
	  
	std::cout << " got: -------- \n";
	for ( auto s : lines ) {
	  std::cout << s << "\n";
	}
	std::cout << " ----------------- \n";

  } // for

  assert (i == N+1);

  sa.close ();

  std::cout << " happy ending ! \n";

  return 0;
}

