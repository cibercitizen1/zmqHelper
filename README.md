zmqHelper
=========

Helper class and functions on top of zmq C++ binding (zmq.hpp).

* Customize example/Makefile.in to your local settings (where your zmq library is).
(Also remember: export
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/pathToYour/zeromq/lib).

* Please explore the examples.

* On Multithreading with ZeroMQ: "If you're sharing sockets across threads, don't. It
will lead to random weirdness, and crashes." Remember: 
 
    - Do not use or close sockets except in the thread that created them.

    - Don't share ZeroMQ sockets between threads. 
      ZeroMQ sockets are not threadsafe. 

    - Isolate data privately within its thread and never share data 
       in multiple threads. The only exception to this are ZeroMQ contexts, 
       which are threadsafe.

    - Stay away from the classic concurrency mechanisms like as mutexes, 
       critical sections, semaphores, etc. These are an anti-pattern 
      in ZeroMQ applications.
 
    - Create one ZeroMQ context at the start of your process, 
      and pass that to all threads that you want to connect via inproc sockets.

* Now, we  enforce the  above rules.  Only the  thread that  created a
socket may use it.  There are  two types of sockets: SocketAdaptor and
SocketAdaptorWithThread.
	- SocketAdaptor checks that the thread calling its methods is the same which created/declared the variable.
	- SocketAdaptorWithThread runs a new thread with exclusive use to an enclosed SocketAdaptor. The constructor expects a function that will be executed by the inner thread.

* Code excerpts

  - REQ client
  	```cpp
SocketAdaptor< ZMQ_REQ > sa;
  sa.connect ("tcp://localhost:5555");

std::vector<std::string> multi = { "Hallo Hallo", "Hello Hello" };
sa.sendText ( multi );

bool recvOK = sa.receiveText (lines); 

sa.close ();
		 ```

  - REP server (with inner thread)
  	```cpp
 SocketAdaptorWithThread< ZMQ_REP > sa
  {	
	[] (SocketAdaptor<ZMQ_REP> &  socket ) -> void { 

	  socket.bind ("tcp://*:5555");
	  
	  while ( socket.receiveText (lines) ) {
	  	std::vector<std::string> lines;
		
		std::cout << " received: ";
		for ( auto s : lines ) { std::cout << s << "\n"; }
		
		// Send the reply
		std::vector<std::string> multi 
		  = { "Welt Welt", "World World" };
		socket.sendText ( multi );
	  } // while
	}  // lambda
  }; // sa
	```
<<<<<<< HEAD
  - broker: ROUTER fronted, DEALER backend (blocking poll made easy)
  	```cpp
  zmq::context_t theContext {1}; // 1 thread in the socket 
  SocketAdaptor< ZMQ_ROUTER > frontend_ROUTER {theContext};
  SocketAdaptor< ZMQ_DEALER > backend_DEALER {theContext};

  frontend_ROUTER.bind ("tcp://*:8000");
  backend_DEALER.bind ("tcp://*:8001");

  while (true) {

        std::vector<std::string> lines;
        
        // 
        //  wait (blocking poll) for data in any socket
        // 
        std::vector< zmqHelper::ZmqSocketType * > list
          = {  frontend_ROUTER.getZmqSocket(),  backend_DEALER.getZmqSocket() };

        zmqHelper::ZmqSocketType *  from = zmqHelper::waitForDataInSockets ( list );

        // 
        //  there is data, where is it from?
        // 
        if ( from ==  frontend_ROUTER.getZmqSocket() ) {
          // from frontend, read ...
          frontend_ROUTER.receiveText (lines);

          // ... and resend
          backend_DEALER.sendText( lines );
        }
        else if ( from ==  backend_DEALER.getZmqSocket() ) {
          // from backend, read ...
          backend_DEALER.receiveText (lines);

          // ... and resend
          frontend_ROUTER.sendText( lines );
		} 
		else if ( from == nullptr ) {
		  std::cerr << "Error in poll ?\n";
		}

  } // while (true)
  ```

  - guest of a chat (REQ for sending, SUB for receiving). Note how
  the inner thread in SUB is notified.
    ```cpp
=======

  - guest of a chat (REQ for sending, SUB for receiving). Note how
  the inner thread in SUB is notified.
  ```cpp
>>>>>>> 56cc41224e71321e7e11418c67d4c4ff33d97309
  bool receiving = true; // for signaling the thread in SUB

  SocketAdaptorWithThread< ZMQ_SUB > receiver { 
	[&receiving] (SocketAdaptor<ZMQ_SUB> & socket ) -> void {
	  socket.connect ("tcp://localhost:8001");
	  socket.subscribe (CHANNEL);
	  
	  while (receiving) {
	  	std::vector<std::string> lines;
		// wait for messages, but only for 1s. Then, awake and 
		// check whether we have to keep on receiving.
		// Otherwise (no timeout), the thread gets blocked in the receive
		// and it never sees any change to 'receiving' var
		// to know it has to end
		if ( ! socket.receiveTextInTimeout (lines, 1000) )  continue;
		
		std::cout << " msg received: |" << std::flush;
		for ( auto s : lines ) { std::cout << s << "|" << std::flush; }
		std::cout << "\n\n" << std::flush;
	  } // while
	  
	  socket.close ();
	}
  };
  //
  //
  //
  SocketAdaptor< ZMQ_REQ > emitter;
  emitter.connect ("tcp://localhost:8000");

  //
  // send first post
  //
  std::vector<std::string> multi = { CHANNEL, NICK, "hi all" };
  emitter.sendText (multi );

  std::vector<std::string> recLines;
  std::string line;

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
<<<<<<< HEAD

  receiving = false; // signal the thread to end
  
  // close socket
  emitter.close ();
=======

  receiving = false;
  
  //
  // close socket
  //
  emitter.close ();
  ```

  - broker: ROUTER fronted, DEALER backend (blocking poll made easy)
  ```cpp
  zmq::context_t theContext {1}; // 1 thread in the socket 
  SocketAdaptor< ZMQ_ROUTER > frontend_ROUTER {theContext};
  SocketAdaptor< ZMQ_DEALER > backend_DEALER {theContext};

  frontend_ROUTER.bind ("tcp://*:8000");
  backend_DEALER.bind ("tcp://*:8001");

  while (true) {

        std::vector<std::string> lines;
        
        // 
        //  wait (blocking poll) for data in any socket
        // 
        std::vector< zmqHelper::ZmqSocketType * > list
          = {  frontend_ROUTER.getZmqSocket(),  backend_DEALER.getZmqSocket() };

        zmqHelper::ZmqSocketType *  from = zmqHelper::waitForDataInSockets ( list );

        // 
        //  there is data, where is it from?
        // 
        if ( from ==  frontend_ROUTER.getZmqSocket() ) {
          // from frontend, read ...
          frontend_ROUTER.receiveText (lines);

          // ... and resend
          backend_DEALER.sendText( lines );
        }
        else if ( from ==  backend_DEALER.getZmqSocket() ) {
          // from backend, read ...
          backend_DEALER.receiveText (lines);

          // ... and resend
          frontend_ROUTER.sendText( lines );
		} 
		else if ( from == nullptr ) {
		  std::cerr << "Error in poll ?\n";
		}

  } // while (true)
>>>>>>> 56cc41224e71321e7e11418c67d4c4ff33d97309
  ```
