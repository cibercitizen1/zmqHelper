// ---------------------------------------------------------------
// guestBetter.cpp
// ---------------------------------------------------------------

#include <zmq.hpp>
#include <string>
#include <iostream>

#include "../../zmqHelper.hpp"

using namespace zmqHelper;

// ---------------------------------------------------------------
// ---------------------------------------------------------------
const std::string NICK = "guest nick 1";
const std::string CHANNEL = "mainChannel";

// -----------------------------------------------------------------
// -----------------------------------------------------------------
int main ()
{

  bool receiving = true; // for signaling the thread in SUB

  SocketAdaptorWithThread< ZMQ_SUB > receiver { 
	[&receiving] (SocketAdaptor<ZMQ_SUB> & socket ) -> void {
	  socket.connect ("tcp://localhost:8001");
	  socket.subscribe (CHANNEL);
	  
	  std::vector<std::string> lines;
	  
	  while (receiving) {
		// wait for messages, but only for 1s. Then, awake and 
		// check whether we have to keep on receiving.
		// Otherwise (no timeout), the thread gets blocked in the receive
		// and it never sees any change to 'receiving' var
		// to know it has to end
		if ( ! socket.receiveTextInTimeout (lines, 1000) )  continue;
		
		std::cout << " msg received: |" << std::flush;
		for ( auto s : lines ) { std::cout << s << "|" << std::flush; }
		std::cout << "\n\n" << std::flush;
	  } // while
	  
	  socket.close ();
	  
	  std::cout << "callback_SUB ended !!!!! \n" << std::flush;
	  
	}
  };

  //
  //
  //

  std::vector<std::string> recLines;
  std::string line;

  //
  //
  //
  SocketAdaptor< ZMQ_REQ > emitter;

  emitter.connect ("tcp://localhost:8000");

  //
  // send first porst
  //
  std::vector<std::string> multi = { CHANNEL, NICK, "hi all" };
  emitter.sendText (multi );

  emitter.receiveText (recLines);
  // ignore recLines (should be "OK")

  do {
	//
	// read ...
	//
	std::cout << " ? " << std::flush;
	getline (std::cin, line);

	//
	// ... and send
	//
	std::vector<std::string> sending = { CHANNEL, NICK, line };
	emitter.sendText ( sending ); 

	emitter.receiveText (recLines);
	// ignore answer (should be "OK")

  } while (line != "BYE" && line != ""); 

  receiving = false;
  
  //
  // close socket
  //
  emitter.close ();
  // receiver.close (); // not this one, not owned by thread-main

} // ()
