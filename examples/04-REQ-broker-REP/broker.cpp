// ---------------------------------------------------------------
// broker.cpp (Thread safe version)
// ---------------------------------------------------------------

#include <zmq.hpp>
#include <string>
#include <iostream>

#include "../../zmqHelper.hpp"

using namespace zmqHelper;

// -----------------------------------------------------------------
// -----------------------------------------------------------------
int main ()
{

  zmq::context_t theContext {1}; // 1 thread in the socket 
  SocketAdaptor< ZMQ_ROUTER > frontend_ROUTER {theContext};
  SocketAdaptor< ZMQ_DEALER > backend_DEALER {theContext};

  // It seems it workstoo, if each socket is having its own context
  /*
  SocketAdaptor< ZMQ_ROUTER > frontend_ROUTER;
  SocketAdaptor< ZMQ_DEALER > backend_DEALER;
  */

  std::cout << " broker: binding ... ";

  frontend_ROUTER.bind ("tcp://*:8000");
  backend_DEALER.bind ("tcp://*:8001");

  std::cout << " done \n";

  //
  //
  //
  while (true) {

        std::vector<std::string> lines;
        
        // 
        //  wait (blocking poll) for data in any socket
        // 
        std::vector< zmqHelper::ZmqSocketType * > list
          = {  frontend_ROUTER.getZmqSocket(),  backend_DEALER.getZmqSocket() };

        zmqHelper::ZmqSocketType *  who = zmqHelper::waitForDataInSockets ( list );

        // 
        //  there is data, where is it from?
        // 
        if ( who ==  frontend_ROUTER.getZmqSocket() ) {
          // 
          // from frontend, read ...
          // 
          frontend_ROUTER.receiveText (lines);

          // 
          // ... and resend
          // 
          backend_DEALER.sendText( lines );

		  std::cout << " ----------->>>>>>> from frontend to backend \n";

        }

        else if ( who ==  backend_DEALER.getZmqSocket() ) {
          // 
          // from backend, read ...
          // 
          backend_DEALER.receiveText (lines);

          // 
          // ... and resend
          // 
          frontend_ROUTER.sendText( lines );

		  std::cout << " <<<<<<------- from backend to frontend \n";
		} 

		else if ( who == nullptr ) {
		  std::cerr << "Error in poll ?\n";
		}

  } // while (true)

} // () main
