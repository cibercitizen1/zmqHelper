// ---------------------------------------------------------------
// client.cpp (use the k-v service and get its publications)
// ---------------------------------------------------------------
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>

// ---------------------------------------------------------------
// ---------------------------------------------------------------
#include "json11.hpp"
#include "../../zmqHelper.hpp"

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  // 
  // socket to send GET, PUT, DELETE actions
  // 
  zmqHelper::SocketAdaptor<ZMQ_REQ> reqSocket;
  reqSocket.connect ("tcp://localhost:5555");
  
  // 
  //  some vars
  // 
  std::vector<std::string> msg;
  bool receiving = true; // to control inner thread

  // 
  //  socket (with inner thread) to get publications 
  // 
  zmqHelper::SocketAdaptorWithThread<ZMQ_SUB> subSocket { 
	[&receiving](zmqHelper::SocketAdaptor<ZMQ_SUB> & socket ) -> void {

	  std::vector<std::string> lines;
  
	  std::cout << " callbak_SUB starts ----- \n" << std::flush;

	  socket.connect ("tcp://localhost:5556");
	  socket.subscribe ("PUT");

	  while ( receiving ) {
		// wait for messages, but only 1s to awake and 
		// check we have to keep receiving
		if ( ! socket.receiveTextInTimeout (lines, 1000) )  continue;
  
		std::cout << " msg received -------------: |";
		for ( auto s : lines ) { std::cout << s << "|"; }
		std::cout << "\n\n" << std::flush;


	  } // while

	  socket.close ();
	} // lambda
  };


  // 
  // send a GET
  // 
  std::vector<std::string> lines {"GET", R"( {"key": "foo"} )"};
  reqSocket.sendText (lines);
  reqSocket.receiveText (msg);

  std::cout << " client received >" << msg[0] << "<\n";
  std::cout << " client received >" << msg[1] << "<\n";

  // 
  // send a PUT
  // 
  lines = {"PUT", R"( {"key": "foo", "value": "barbarbar"} )"};
  reqSocket.sendText ( lines );
  reqSocket.receiveText (msg);

  std::cout << " client received >" << msg[0] << "<\n";
  std::cout << " client received >" << msg[1] << "<\n";

  // 
  // send a GET
  // 
  lines = {"GET", R"( {"key": "foo"} )"};
  reqSocket.sendText (lines);

  reqSocket.receiveText (msg);

  std::cout << " client received >" << msg[0] << "<\n";
  std::cout << " client received >" << msg[1] << "<\n";

  // 
  // parse the JSON
  // 
  std::string jsonErr;
  auto arguments = json11::Json::parse ( msg[1], jsonErr);
  auto key = arguments["key"].string_value();
  auto value = arguments["value"].string_value();
  assert ( key == "foo" && value == "barbarbar" );

  // 
  // stop the thread in sub socket
  // 
  receiving = false;
  //   sleep (1);

  // 
  // 
  // 
  reqSocket.close ();

  std::cout << " that's all, folks ! \n" << std::flush;

} // main ()
