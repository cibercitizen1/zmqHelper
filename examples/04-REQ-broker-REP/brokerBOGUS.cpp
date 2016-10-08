// ---------------------------------------------------------------
// broker.cpp (NOT THREAD SAFE VERSION)
// ---------------------------------------------------------------

#include <zmq.hpp>
#include <string>
#include <iostream>

#include "../../zmqHelper.hpp"

using namespace zmqHelper;

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

// -----------------------------------------------------------------
// -----------------------------------------------------------------
int main ()
{

  zmq::context_t theContext {1}; // 1 thread in the socket 
  SocketAdaptor< ZMQ_ROUTER > frontend_ROUTER {theContext};
  SocketAdaptor< ZMQ_DEALER > backend_DEALER {theContext};

  frontend_ROUTER.bind ("tcp://*:8000");
  backend_DEALER.bind ("tcp://*:8001");

  // 
  // WARNING: this code seems to run fine for
  // the simple case but it is bogus according
  // to Zmq rules
  // 
  frontend_ROUTER.onMessage ( [&] (SocketAdaptor<ZMQ_ROUTER> & socket ) {

	  // 
	  // BUG 1. The thread here is different of the thread that
	  // created the sockets (thread-main).
	  // 
	  // BUG 2. The thread here is accessing two sockets, shared
	  // with the thread on the other callback
	  // 
	  auto lines = socket.receiveText ();
	  
	  std::cout << " msg received on FRONTEND = |"; 
	  for ( auto s : lines ) { std::cout << s << "(" << s.size() << ")|"; }
	  std::cout << "\n";

	  // routing
	  backend_DEALER.sendText (lines);
	 
	} );

  // 
  backend_DEALER.onMessage ( [&] (SocketAdaptor<ZMQ_DEALER> & socket ) {
	  // 
	  // BUG 1. The thread here is different of the thread that
	  // created the sockets (thread-main).
	  // 
	  // BUG 2. The thread here is accessing two sockets, shared
	  // with the thread on the other callback
	  // 
	  auto lines = backend_DEALER.receiveText ();
	  
	  std::cout << " msg received on BACKEND = |"; 
	  for ( auto s : lines ) { std::cout << s << "(" << s.size() << ")|"; }
	  std::cout << "\n";

	  // routing
	  frontend_ROUTER.sendText (lines);
	 
	} );

  // never happens because we don't stop the receivers
  frontend_ROUTER.wait ();
  backend_DEALER.wait ();

} // () main
