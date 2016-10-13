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

  // caution: member fields are initialized in this order
  // as they appear defined (not as the are listed in the constructor)
  // as theSocket depend on myContext: myContext MUST appear BEFORE
  
  zmq::context_t myContext; // not constructed now

  SocketAdaptorWithThread< ZMQ_REP > theSocket; 
  
  const std::string bankName;
  

public:

  // .............................................................
  // .............................................................
  const std::string & getName () const {
	return bankName;
  }

  // .............................................................
  // .............................................................
  Bank (const std::string & bn, std::function<void(SocketAdaptor<ZMQ_REP> &)> f)  
	: myContext { 1 }, // constructed now
	  bankName (bn),
	  theSocket { myContext, f }
  {

	std::cerr << " Bank constructor: done \n";
  } 
  
  // ..............................................................
  // ..............................................................
  zmq::context_t & getContext () {
	std::cerr << " Bank getContext: called \n";
	return myContext;
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

	std::cerr << " thread of person " << personName << " joined \n";
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


  std::cout << " ***\t personRole of " << name << " starting \n" << std::flush;
	
  // Two threads communicate over inproc, using a shared context.
  SocketAdaptor<ZMQ_REQ> theSocket { bank.getContext() };

  std::cout << " ***\t personRole of " << name << " socket created \n";

  std::vector<std::string> lines;
  
  std::cout << " ***\t personRole of " << name << " going to connect \n";
  std::string url = "inproc://bank" + bank.getName();
  theSocket.connect (url);
  
  std::cout << " ***\t personRole of " << name << " going to send \n";
  
  for (int i=1; i<=5; i++) {
	
	std::cout << name << " ***\t person sending \n";
	
	// send request
	std::vector<std::string> multi = { name, "put", "12.34" };
	theSocket.sendText (multi);
	
	//  get reply
	theSocket.receiveText (lines);
	
	std::cout << name << " ***\t received:|";
	for ( auto s : lines ) { std::cout << s << "|"; }
	std::cout << "\n";
	
	sleep (1);
  } // for
  
  theSocket.close ();
}; // ()

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  std::string bankName = "SwissBankers";

  bool doorsOpen = true;

  auto bankRole =  [&]  (SocketAdaptor<ZMQ_REP> & socket ) -> void { 

	sleep (2); // slowish banker

	std::cout << " bankRole of " << bankName << " starting \n";
	
	std::vector<std::string> lines;

	socket.bind ( "inproc://bank" + bankName );

	std::cerr << " \t\t\t\t\t banckRole, receiving ... \n";

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

  /* 
   * I had a race condition:
   * The Bank constructor calls the Socket constructor giving it the context.
   * The thread of the zmq_socket suspends before it properly saves the context,
   * but the main-thread goes on, and starts the Persons
   * A Thread of a person asks to get the context of Bank (which, in turn, 
   * !!! in the previous zmqHelper version !!!
   * asks the socket for the context. An invalid context is returned.
   * Thus, when the person constructs its socket the program aborts
   * (terminating with uncaught exception of type zmq::error_t: Bad address)
   * and share it.
   * Solved, restricting the use of SockedAdaptor:getSocket() and
   * creating and sharing the context externally to the sockets.
   */

  Person p1 ( "Alice" );
  Person p2 ( "Bob" );

  // curry personRole() from 2 args to 1 arg ( let bank be b1 )
  auto role = [&] (const std::string & s) { return personRole (b1, s); };

  p1.act (role);
  p2.act (role);

  std::cout << " +++ main: waiting for person threads \n";

  p1.join ();
  p2.join ();

  std::cout << " +++ main: person threads ended, closing doors \n";
  doorsOpen = false;

  // std::cerr << " main thread = " << std::this_thread::get_id() << "\n";

  std::cout << " ================ happy ending ==================\n" << std::flush;

  return 0;

} // main ()

