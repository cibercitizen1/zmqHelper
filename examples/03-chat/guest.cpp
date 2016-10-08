// ---------------------------------------------------------------
// guest.cpp
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
void callback_REQ (SocketAdaptor<ZMQ_REQ> & socket ) {

  // BUG: two threads using the same socket
  // one sends the other one receives.
  // 
  // The thread executing this reception
  // is different than the one in main
  // sending messages.
  // Apparently it works fine, for trivial tests.

  auto lines = socket.receiveText ();
  // ignore the lines (should be "OK")
  
} // ()

// ---------------------------------------------------------------
// ---------------------------------------------------------------
void callback_SUB (SocketAdaptor<ZMQ_SUB> & socket ) {

  // 
  // WARNING:
  // Here, the thread receiving is also different
  // from the one in main, but only this thread
  // is using the socket: no contention.
  // 

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

  emitter.onMessage ( callback_REQ );
  receiver.onMessage ( callback_SUB );

  // send first porst
  std::vector<std::string> multi = { CHANNEL, NICK, "hi all" };
  emitter.sendText (multi );

  // read and send
  std::string line;
  do {
	std::cout << " ? ";
	getline (std::cin, line);

	std::vector<std::string> sending = { CHANNEL, NICK, line };
	emitter.sendText ( sending ); 

  } while (line != "BYE"); 
  
  // stop threads
  emitter.stopReceiving ();
  receiver.stopReceiving ();

  std::cout << " happy ending \n";

} // ()
