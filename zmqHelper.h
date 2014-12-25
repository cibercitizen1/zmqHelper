/*
 * -----------------------------------------------------------------
 * zmqHelper.h
 * -----------------------------------------------------------------
 */

#ifndef ZQM_HELPER_H
#define ZQM_HELPER_H

// -----------------------------------------------------------------
// -----------------------------------------------------------------
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <vector>

#include <thread>        

namespace zmqHelper {

  // -----------------------------------------------------------------
  // -----------------------------------------------------------------
  // bool hasMore (zmq::socket_t & socket) {
  template<typename SocketType>
	bool hasMore (SocketType & socket) {
	int64_t more = 0;           //  Multipart detection
	size_t more_size = sizeof (more);
	socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
  
	/*
	  std::cerr << " more: " << more << " " << more_size << "\n";
	  if ( more ) {
	  std::cerr << " more parts\n";
	  } else {
	  std::cerr << " NO more parts\n";
	  }
	*/
  
	return more;
  } // ()

  // ---------------------------------------------------------------
  // ---------------------------------------------------------------
  template<typename SocketType>
	void sendText (SocketType & socket, const std::string & msg) {
	zmq::message_t reply (msg.size());
	memcpy ((void *) reply.data (), msg.c_str(), msg.size());
	socket.send (reply);
  }

  // ---------------------------------------------------------------
  // ---------------------------------------------------------------
  template<typename SocketType>
	void sendText (SocketType & socket, const std::vector<std::string> & msgs) {

	unsigned int many = msgs.size ();
	unsigned int i=1;
	for (auto msg : msgs) {
	  zmq::message_t reply (msg.size());
	  memcpy ((void *) reply.data (), msg.c_str(), msg.size());

	  int more = i<many ? ZMQ_SNDMORE : 0;
	  // std::cerr << " sending part " << i << "more = " << more << "\n";
	  socket.send (reply, more);
	  i++;
	}
  }

  // ---------------------------------------------------------------
  // ---------------------------------------------------------------
  template<typename SocketType>
	const std::vector<std::string> receiveText (SocketType & socket) {

	std::vector<std::string> result;

	do {
	  zmq::message_t reply;
	  socket.recv (&reply);

	  // char buff[100];
	  // memcpy (buff, reply.data(), reply.size());
	  result.push_back ( std::string { (char*) reply.data(), reply.size() } );

	} while ( hasMore( socket ) );

	return result;

  } // ()

  // ---------------------------------------------------------------
  // ---------------------------------------------------------------
  using SocketType = zmq::socket_t;
  using FunctionType = std::function<void(SocketType&)>;

  // ---------------------------------------------------------------
  // ---------------------------------------------------------------
  template<int SOCKET_TYPE>
	class SocketAdaptor {
  private:
  
	// .............................................................
	FunctionType theCallback;
	bool callbackValid = false;

	// .............................................................
	std::thread * theThread = 0;
 
	// .............................................................
	zmq::context_t context {1};
	SocketType  theSocket {context, SOCKET_TYPE};

	// .............................................................
	// .............................................................
	void main () {

	  // std::cerr << " main starts \n";

	  while ( callbackValid ) {
	  
		// poll to know when data has arrived
		zmq::pollitem_t items [] = {
		  { theSocket, 0, ZMQ_POLLIN, 0}
		};
		// zmq::poll ( &items[0], 1, -1);
		int some = zmq::poll ( &items[0], 1, 200); // timeout=200ms, some>0 => hay algo
	  
		// data has arrived, call the handler
		if ( some > 0 && callbackValid ) theCallback (theSocket);
	  } // while

	  // std::cerr << " main acaba \n";
	} // ()

	// .............................................................
	// .............................................................
	SocketAdaptor (const SocketAdaptor & o)  { 
	  // std::cerr << " forbidden  copy constructor\n";
	}

	// .............................................................
	// .............................................................
	SocketAdaptor & operator=(const SocketAdaptor & o)  { 
	  // std::cerr << " forbidden  assignment \n";
	}

  public:

	// .............................................................
	// .............................................................
	SocketAdaptor ()  { 
	  // std::cerr << " constructor \n";
	}

	// .............................................................
	// .............................................................
	~SocketAdaptor ()  { 
	  // std::cerr << " destructor \n";
	  stopReceiving (); 
	}

	// .............................................................
	// .............................................................
	void bind (const std::string & url)  { 
	  theSocket.bind (url.c_str());
	}

	// .............................................................
	// .............................................................
	void connect (const std::string & url)  { 
	  theSocket.connect (url.c_str());
	}

	// .............................................................
	// .............................................................
	void disconnect (const std::string & url)  { 
	  theSocket.disconnect (url.c_str());
	}

	// .............................................................
	// .............................................................
	void subscribe (const std::string & filter)  { 
	  theSocket.setsockopt(ZMQ_SUBSCRIBE, filter.c_str(), filter.size());
	}

	// .............................................................
	// .............................................................
	SocketType & getSocket () {
	  return theSocket;
	}

	// .............................................................
	// .............................................................
	void onMessage (FunctionType  f) {
   
	  if (callbackValid) {
		// running, must first call stop()
		return;
	  }
   
	  // std::cerr << " start  begins \n";
	  theCallback = f;
	  callbackValid = true;

	  theThread = new std::thread ( [this] () { main(); } );
	}

	// .............................................................
	// .............................................................
	void stopReceiving () {
	  if (! callbackValid) {
		// not running, nothing to do
		return;
	  }

	  // std::cerr << " stopping thread \n";

	  // this stops the thread loop
	  callbackValid = false;

	  wait ();
	}

	// .............................................................
	// .............................................................
	void wait () {
	  if ( theThread == 0 ) {
		// not running, nothing to do
		return;
	  }

	  // std::cerr << " waiting thread \n";

	  theThread->join ();
	  delete (theThread);
	  theThread = 0;

	  // std::cerr << " waiting thread DONE \n";
	
	}

	// .............................................................
	// .............................................................
	void sendText (const std::vector<std::string> & msgs) {
	  zmqHelper::sendText (theSocket, msgs);
	}

	// .............................................................
	// .............................................................
	const std::vector<std::string> receiveText () {
	  return zmqHelper::receiveText (theSocket);
	}

	// .............................................................
	// .............................................................
	const void close () {
	  theSocket.close ();
	}
	
  };

};

#endif
