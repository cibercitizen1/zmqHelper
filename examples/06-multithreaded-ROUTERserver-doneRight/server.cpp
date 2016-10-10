
// ---------------------------------------------------------------
// server.cpp
// 
//  multi-threaded router server done right !
// 
// 
//  -> ROUTER socket -> main-thread -> in-proc socket -> delegate to worker
// 
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
 *
 *       Stay away from the classic concurrency mechanisms like as mutexes, 
 *       critical sections, semaphores, etc. These are an anti-pattern 
 *       in ZeroMQ applications.
 * 
 *       Create one ZeroMQ context at the start of your process, 
 *       and pass that to all threads that you want to connect via inproc sockets.
 *
 *
 *
 * If you need to start more than one proxy in an application, 
 * for example, you will want to run each in their own thread. 
 * It is easy to make the error of creating the proxy frontend 
 * and backend sockets in one thread, and then passing the sockets 
 * to the proxy in another thread. This may appear to work at first 
 * but will fail randomly in real use. 
 *
 * Some widely used models, despite being the basis for entire 
 * industries, are fundamentally broken, and shared state concurrency 
 * is one of them. Code that wants to scale without limit does it 
 * like the Internet does, by sending messages and sharing nothing
 *
 */

/*
 *
 * ON CONTEXTS
 *
 * ZeroMQ applications always start by creating a context, 
 * and then using that for creating sockets. 
 * In C, it's the zmq_ctx_new() call. 
 * You should create and use exactly one context in your process. 
 * Technically, the context is the container for all sockets 
 * in a single process, and acts as the transport for inproc sockets, 
 * which are the fastest way to connect threads in one process. 
 * If at runtime a process has two contexts, 
 * these are like separate ZeroMQ instances. 
 * If that's explicitly what you want, OK, but otherwise remember:
 * 
 * Do one zmq_ctx_new() at the start of your main line code, 
 * and one zmq_ctx_destroy() at the end.
 */
// ---------------------------------------------------------------
#include <iostream>
#include <future>
#include <thread>

#include <string>
#include <vector>
#include <stdlib.h>

#include "../../zmqHelper.hpp"


// ---------------------------------------------------------------
// ---------------------------------------------------------------
const std::string PORT_NUMBER = "5580";

// ---------------------------------------------------------------
// ---------------------------------------------------------------
template <typename ITERABLE>
void showLines (const std::string & msg, ITERABLE & it) {
  std::cout <<  msg << ":  message: |";
  for (auto item : it) {
	std::cout << item << "|";
  }
  std::cout << "\n";
}

// ---------------------------------------------------------------
// ---------------------------------------------------------------

// ---------------------------------------------------------------
// ---------------------------------------------------------------
class Worker {

private:

  // 
  // 
  // 
  zmq::context_t & sharedContext;

  // 
  // previous version: the thread in constructor creates
  // the socket but it is used by the thread in main
  // 
  // zmqHelper::SocketAdaptor< ZMQ_REP > internalRepSocket;

  // 
  // 
  // 
  std::thread * theThread = nullptr;
 
  // -------------------------------------------------------------
  // -------------------------------------------------------------
  void main (){
	//
	// a new thread executes this
	// 

	//
	// the socket is created and use here
	//
	// remember RAII (resourse adquisition is initialization)
	// 
	zmqHelper::SocketAdaptor< ZMQ_REP > internalRepSocket {sharedContext};

	//
	//
	//
	internalRepSocket.connect ("inproc://innerChannel");

	//
	//
	// 
	std::cout << " ***** worker main " << this
			  << " starts **** \n";
	
	//
	//
	// 
	while (true) {
	  std::cout << " worker waiting for socket \n";
	  auto lines = internalRepSocket.receiveText ();

	  std::cout << " ** worker: " << this;
	  showLines (" worker ", lines);

	  //
	  // do some work !
	  //
	  sleep (1);

	  // 
	  //  reply
	  // 
	  internalRepSocket.sendText ("echo of: " + lines[0] + " " + lines[1] );
	  
	} // while
  } // ()

public:

  // -------------------------------------------------------------
  // -------------------------------------------------------------
  Worker ( zmq::context_t & sharedContext_) :
	sharedContext {sharedContext_}
  {
	//
	//
	// 
	theThread = new std::thread (&Worker::main, this);
	
  }

  // -------------------------------------------------------------
  // -------------------------------------------------------------
  ~Worker ( ) {
	std::cerr << " worker destructor \n";
	if (theThread == nullptr) {
	  return;
	}
	
	theThread->join ();
	delete (theThread);
	theThread = nullptr;
  }

};

// ---------------------------------------------------------------
// ---------------------------------------------------------------
int main () {

  std::cout << " main stars \n";

  //
  // create internal socket to talk to workers
  //
  zmqHelper::SocketAdaptor< ZMQ_DEALER > innerDealerSocket; 
  innerDealerSocket.bind ("inproc://innerChannel");

  //
  // 4 workers, you can experience with the
  // number of workers, timing run.client
  //
  Worker wk1 {innerDealerSocket.getContext()};
  Worker wk2 {innerDealerSocket.getContext()};
  Worker wk3 {innerDealerSocket.getContext()};
  Worker wk4 {innerDealerSocket.getContext()};

  std::cerr << "main(): workers created\n";

  //
  // create external socket to talk with clients
  //
  zmqHelper::SocketAdaptor< ZMQ_ROUTER > outerRouterSocket; 
  outerRouterSocket.bind ("tcp://*:" + PORT_NUMBER);

  //
  // for ever ...
  //
  while (true) {

	std::cout << "\n\n while(true): waiting for clients ... \n";

	std::vector<std::string>  lines;

	// 
	//  wait (blocking poll) for data in either of the sockets
	// 
	std::vector< zmqHelper::ZmqSocketType * > list
	  = {  outerRouterSocket.getZmqSocket(),  innerDealerSocket.getZmqSocket() };

	zmqHelper::ZmqSocketType *  who = zmqHelper::waitForDataInSockets ( list );

        // 
        //  there is data
        // 
        if ( who ==  outerRouterSocket.getZmqSocket() ) {
          // 
          // from client
          // 
          lines = outerRouterSocket.receiveText ();

          // 
          // delegate to worker
          // 
          innerDealerSocket.sendText( lines );

        }
        else if ( who ==  innerDealerSocket.getZmqSocket() ) {
          // 
          // reply from worker
          // 
          lines = innerDealerSocket.receiveText ();

          // 
          //  answer the client
          // 
          outerRouterSocket.sendText( lines );
        } else {
          std::cout << " server: error in polling? \n";
        }

  } // while

} // main ()

