// ---------------------------------------------------------------
// bank.cpp
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

#include <string>
#include <vector>
#include <stdlib.h>

#include <thread>        

#include "../../zmqHelper.hpp"

using namespace zmqHelper;

/*
 * Two threads communicate over inproc, using a shared context.
 */

// ---------------------------------------------------------------
// ---------------------------------------------------------------
class Bank {

private:

  SocketAdaptor< ZMQ_REP > theSocket; 
  
  const std::string bankName;

public:

  // .............................................................
  // .............................................................
  using SocketType = decltype (theSocket);

  // .............................................................
  // .............................................................
  const std::string & getName () const {
	return bankName;
  }

  // .............................................................
  // .............................................................
  Bank (const std::string & bn, std::function<void(SocketType &)> f)  
	: bankName (bn) ,
	  theSocket { f }
  {
  } 
  
  // ..............................................................
  // ..............................................................
  zmq::context_t & getContext () {
	return theSocket.getContext ();
  }

  // .............................................................
  // .............................................................
  ~Bank () {
	theSocket.joinTheThread ();
  }
};

// ---------------------------------------------------------------
// ---------------------------------------------------------------
class Person {

private:

  // .............................................................
  std::string personName;

  // .............................................................
  std::thread * theThread = nullptr;

public:

  // .............................................................
  // .............................................................
  Person (const std::string & pn) : 
	personName {pn}
  { }

  // .............................................................
  // .............................................................
  void act (std::function<void(const std::string &)> f ) {
	if ( theThread != nullptr) {
	  return;
	}

	theThread = new std::thread ( f, personName );
  } // ()

  // .............................................................
  // .............................................................
  void join () {
	if (theThread == nullptr) {
	  return;
	}

	theThread->join ();
	delete (theThread);
	theThread = nullptr;

  } // ()

  // .............................................................
  // .............................................................
  ~Person () {
	join (); // sure?
  }
};

// ---------------------------------------------------------------
// ---------------------------------------------------------------
void personRole (Bank & bank, const std::string & name)  {
	
	// Two threads communicate over inproc, using a shared context.
	auto theSocket = new SocketAdaptor<ZMQ_REQ> { bank.getContext() };

	std::vector<std::string> lines;
	
	std::string url = "inproc://bank" + bank.getName();
	theSocket->connect (url);

	for (int i=1; i<=5; i++) {

	  std::cout << name << " sending \n";

	  // send request
	  std::vector<std::string> multi = { name, "put", "12.34" };
	  theSocket->sendText (multi);

	  //  get reply
	  theSocket->receiveText (lines);
	  
	  std::cout << name << " received:| ";
	  for ( auto s : lines ) { std::cout << s << "|"; }
	  std::cout << "\n";

	  sleep (1);
	} // for
	
	theSocket->close ();
	delete theSocket;
  }; // ()

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  std::string bankName = "SwissBankers";

  bool doorsOpen = true;

  auto bankRole =  [&]  (SocketAdaptor<ZMQ_REP> & socket ) -> void { 

	
	std::vector<std::string> lines;

	socket.bind ( "inproc://bank" + bankName );

	while (doorsOpen) { 
	  if (! socket.receiveTextInTimeout (lines, 500) ) continue;

	  /*
	   * while (doorsOpen && socket.receiveText (lines) ) {
	   * This way, the thread gets stalled on receiving
	   * and even closing the socket (by a different thread)
	   * can't awake it. So we use a the receive with timeout
	   */
	
	  std::cout << bankName << " received: |";
	  for ( auto s : lines ) { std::cout << s << "|"; }
	  std::cout << "\n";
	  
	  // Send the reply
	  std::vector<std::string> multi = { "OK", lines[0] };
	  socket.sendText ( multi );

	  std::cerr << " \t\t\t\t\t banckRole, receiving ... \n";
	} // while

	socket.close ();

	std::cerr << " bankRole ended \n";

  };

  Bank b1 (bankName, bankRole);

  Person p1 ( "john" );
  Person p2 ( "mary" );

  // curry personRole() from 2 args to 1 arg ( let bank be b1 )
  auto role = [&] (const std::string & s) { return personRole (b1, s); };

  p2.act (role);
  p1.act (role);

  std::cout << " main waits for person threads \n";

  p1.join ();
  p2.join ();

  std::cout << " person threads ended, closing doors \n";
  doorsOpen = false;

  // std::cerr << " main thread = " << std::this_thread::get_id() << "\n";

  std::cout << " ================ happy ending ==================\n" << std::flush;

  return 0;
} // main ()

