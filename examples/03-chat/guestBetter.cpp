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

// ---------------------------------------------------------------
// ---------------------------------------------------------------
void callback_SUB (SocketAdaptor<ZMQ_SUB> & socket ) {

  // CAUTION:
  // The thread here (receiving) is different
  // from the one in main, but only the thread
  // here is using this socket

  auto lines = socket.receiveText ();
  
  std::cout << " msg received ------------- |";
  for ( auto s : lines ) { std::cout << s << "|"; }
  std::cout << "\n\n";

} // ()

// -----------------------------------------------------------------
int main ()
{

  SocketAdaptor< ZMQ_REQ > emitter;
  SocketAdaptor< ZMQ_SUB > receiver;

  emitter.connect ("tcp://localhost:8000");

  receiver.connect ("tcp://localhost:8001");
  receiver.subscribe (CHANNEL);

  receiver.onMessage ( callback_SUB );

  // send first porst
  std::vector<std::string> multi = { CHANNEL, NICK, "hi all" };
  emitter.sendText (multi );
  auto lines = emitter.receiveText ();
  // ignore the lines (should be "OK")
  
  // read and send
  std::string line;
  do {
	std::cout << " ? ";
	getline (std::cin, line);

	std::vector<std::string> sending = { CHANNEL, NICK, line };
	emitter.sendText ( sending ); 

	lines = emitter.receiveText ();
	// ignore the lines (should be "OK")

  } while (line != "BYE"); 
  
  // stop threads
  emitter.stopReceiving ();
  receiver.stopReceiving ();

  std::cout << " happy ending \n";

} // ()
