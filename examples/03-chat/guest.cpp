// ---------------------------------------------------------------
// guestBetter.cpp
// ---------------------------------------------------------------

/*
* ON MULTITHREADING WITH ZeroMQ
*
* Remember: 
*
*       Do not use or close sockets except in the thread that created them.
*
*       Don't share ZeroMQ sockets between threads. 
*       ZeroMQ sockets are not threadsafe. 
* 
*       Isolate data privately within its thread and never share data 
*       in multiple threads. The only exception to this are ZeroMQ contexts, 
*       which are threadsafe.
*/

#include <zmq.hpp>
#include <string>
#include <iostream>

#include "../../zmqHelper.hpp"

using namespace zmqHelper;

// ---------------------------------------------------------------
// ---------------------------------------------------------------
const std::string NICK = "fooBar";
const std::string CHANNEL = "mainChannel";

// -----------------------------------------------------------------
// -----------------------------------------------------------------
int main ()
{

  std::vector<std::string> recLines;
  std::string line;

  bool receiving = true;

  SocketAdaptor< ZMQ_REQ > emitter;
  SocketAdaptor< ZMQ_SUB > receiver { 
	[&receiving] (SocketAdaptor<ZMQ_SUB> & socket ) -> void {
	  socket.connect ("tcp://localhost:8001");
	  socket.subscribe (CHANNEL);
	  
	  std::vector<std::string> lines;
	  
	  while (receiving) {
		if ( ! socket.receiveText (lines) ) break;
		
		std::cout << " msg received: |" << std::flush;
		for ( auto s : lines ) { std::cout << s << "|" << std::flush; }
		std::cout << "\n\n" << std::flush;
	  } // while
	  
	  socket.close ();
	  
	  std::cout << "callback_SUB ended !!!!! \n" << std::flush;
	  
	}
  };

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
