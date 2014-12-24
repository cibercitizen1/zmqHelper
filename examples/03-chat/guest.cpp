// ---------------------------------------------------------------
// guest.cpp
// ---------------------------------------------------------------

#include <zmq.hpp>
#include <string>
#include <iostream>

#include "../../zmqHelper.h"

using namespace zmqHelper;

// ---------------------------------------------------------------
// ---------------------------------------------------------------
const std::string NICK = "fooBar";
const std::string CHANNEL = "mainChannel";

// ---------------------------------------------------------------
// ---------------------------------------------------------------
void callback_REP (zmq::socket_t & socket ) {

  auto lines = receiveText (socket);

  // ignore the lines (should be "OK")
  
} // ()

// ---------------------------------------------------------------
// ---------------------------------------------------------------
void callback_SUB (zmq::socket_t & socket ) {

  auto lines = receiveText (socket);
  
  std::cout << " msg received -------------: ";
  for ( auto s : lines ) { std::cout << s << " | "; }
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

  emitter.onMessage ( callback_REP );
  receiver.onMessage ( callback_SUB );

  // send first porst
  auto & socket = emitter.getSocket ();

  std::vector<std::string> multi = { CHANNEL, NICK, "hi all" };
  sendText ( socket, multi );

  // read and send
  std::string line;
  do {
	getline (std::cin, line);

	std::vector<std::string> sending = { CHANNEL, NICK, line };
	
	emitter.sendText ( sending ); 

  } while (line != "BYE"); 
  
  // stop threads
  emitter.stopReceiving ();
  receiver.stopReceiving ();

  std::cout << " happy ending \n";

} // ()
