// ---------------------------------------------------------------
// server.cpp (key-value service with publish)
// ---------------------------------------------------------------
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>

#include <map>


// ---------------------------------------------------------------
// ---------------------------------------------------------------
#include "json11.hpp"
#include "../../zmqHelper.hpp"

// ---------------------------------------------------------------
// ---------------------------------------------------------------
const std::string REP_PORT = "5555";
const std::string PUB_PORT = "5556";

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  std::map<std::string, std::string> theKeyValueData;
  theKeyValueData["foo"] = "bar";

  zmqHelper::SocketAdaptor<ZMQ_PUB> pubSocket;
  pubSocket.bind ("tcp://*:" + PUB_PORT);

  zmqHelper::SocketAdaptor<ZMQ_REP> repSocket;
  repSocket.bind ("tcp://*:" +  REP_PORT);

  std::cout << " --------------------------------- \n";
  std::cout << " key-value service with publish \n";
  std::cout << " REP port = " << REP_PORT << "\n";
  std::cout << " PUB port = " << PUB_PORT << "\n";
  std::cout << " --------------------------------- \n";

  std::vector<std::string> msg;

  //
  // while
  //
  while (repSocket.receiveText(msg)) {

	//
	// extract data from message
	//
	auto functionName = msg[0];
	std::string jsonErr;

	std::cout << " functionName: " << functionName << " arguments: " << msg[1] << "\n";

	//
	// arguments are in JSON, parse msg[1] and extract
	//
	auto arguments = json11::Json::parse ( msg[1], jsonErr);
	auto key = arguments["key"].string_value();
	auto value = arguments["value"].string_value();

	std::vector<std::string> textResponse;

	//
	// we support GET, PUT, and DELETE
	//
	if ( functionName == "GET") {
	  std::cout << " GET \n";
	  textResponse = {"null", 
					  "{ \"key\": \"" + key + "\",  \"value\": \"" + theKeyValueData[key] + "\" }" 
		};
	  repSocket.sendText ( textResponse );
	}

	else if ( functionName == "PUT") {
	  std::cout << " PUT \n";
	  theKeyValueData[key] = value;
	  textResponse =  {"null", "{\"ok\": \"true\"}" };
	  repSocket.sendText ( textResponse );
		
	  // publish the new pair key-value
	  std::cout << " publishing the PUT : " << msg[1] << "\n";
	  textResponse = {"PUT", msg[1]};
	  pubSocket.sendText ( textResponse );
	  } 

	else if ( functionName == "DELETE") {
	  std::cout << " DELETE \n";
	  theKeyValueData.erase(key);
	  textResponse = {"null", "{\"ok\": \"true\"}" };
	  repSocket.sendText ( textResponse );
	} 

	else {
	  std::cout << " unsupported \n";
	  textResponse= {"unsupported", "null" };
	  repSocket.sendText ( textResponse );
	}

  } // while

  repSocket.close ();
  pubSocket.close ();
  
} // main ()
