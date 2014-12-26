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
#include <mutex>

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
  template<int SOCKET_TYPE> class SocketAdaptor; 

  // ---------------------------------------------------------------
  // ---------------------------------------------------------------
  using SocketType = zmq::socket_t;

  template<int SOCKET_TYPE>
  using FunctionType = std::function<void(SocketAdaptor<SOCKET_TYPE>&)>;

  // ---------------------------------------------------------------
  // ---------------------------------------------------------------
  template<int SOCKET_TYPE>
	class SocketAdaptor {

  private:
  
	// .............................................................
	FunctionType<SOCKET_TYPE> theCallback;
	bool callbackValid = false;

	// .............................................................
	std::thread * theThread = nullptr;

	// .............................................................
	std::mutex theMutex;
	// std::unique_lock<std::mutex> theLock {theMutex, std::defer_lock}; 
#define LOCK std::unique_lock<std::mutex> theLock {theMutex};
 
	// .............................................................
	zmq::context_t theContext {1};
	SocketType  theSocket {theContext, SOCKET_TYPE};

	// .............................................................
	// .............................................................
	void main () {

	  // std::cerr << " main starts \n";

	  while ( callbackValid ) {

		// 
		// just poll to see if data has arrived !
		// 
		zmq::pollitem_t items [] = { { theSocket, 0, ZMQ_POLLIN, 0} };
		int some = zmq::poll ( &items[0], 1, 200); // timeout=200ms, some>0 => something arrived
		// zmq::poll ( &items[0], 1, -1); // -1 = blocking
	  
		// 
		// data has arrived, call the handler
		// 
		if ( some > 0 && callbackValid ) theCallback (*this);

	  } // while

	  // std::cerr << " main ends \n";
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
	SocketAdaptor (zmq::context_t & aContext ) : theSocket {aContext, SOCKET_TYPE}
	{ }

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
	void onMessage (FunctionType<SOCKET_TYPE>  f, bool startThread=true) {
   
	  if (callbackValid) {
		// running, must first call stopReceiving()
		return;
	  }

	  theCallback = f;
	  callbackValid = true;

	  // start a thread if required, else just call main()
	  if ( startThread ) {
		theThread = new std::thread ( [this] () { main(); } );
	  } else {
		theThread = nullptr;
		main ();
	  }

	} // ()

	// .............................................................
	// .............................................................
	void receiveMessages (FunctionType<SOCKET_TYPE>  f) {
	  onMessage (f, false);
	} // ()

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
	  if ( theThread == nullptr ) {
		// not running, nothing to do
		return;
	  }

	  // std::cerr << " waiting thread \n";

	  theThread->join ();
	  delete (theThread);
	  theThread = nullptr;

	  // std::cerr << " waiting thread DONE \n";
	
	}

	// .............................................................
	// .............................................................
	void sendText (const std::vector<std::string> & msgs) {
	  
	  LOCK; // theLock.lock();

	  unsigned int many = msgs.size ();
	  unsigned int i=1;
	  for (auto msg : msgs) {
		zmq::message_t reply (msg.size());
		memcpy ((void *) reply.data (), msg.c_str(), msg.size());
		
		int more = i<many ? ZMQ_SNDMORE : 0;
		// std::cerr << " sending part " << i << "more = " << more << "\n";
		theSocket.send (reply, more);
		i++;
	  }

	  // theLock.unlock();
	} // ()

	// .............................................................
	// .............................................................
	void sendText (const std::string & msg) {

	  LOCK; // theLock.lock();

	  zmq::message_t reply (msg.size());
	  memcpy ((void *) reply.data (), msg.c_str(), msg.size());
	  theSocket.send (reply);

	  // theLock.unlock();
	}

	// .............................................................
	// .............................................................
	const std::vector<std::string> receiveText () {

	  LOCK; // theLock.lock();

	  std::vector<std::string> result;

	  do {
		zmq::message_t reply;
		theSocket.recv (&reply);

		// char buff[100];
		// memcpy (buff, reply.data(), reply.size());
		result.push_back ( std::string { (char*) reply.data(), reply.size() } );
		
	  } while ( hasMore( theSocket ) );
	  
	  // theLock.unlock();

	  return result;
	  
	} // ()

	// .............................................................
	// .............................................................
	const void close () {
	  theSocket.close ();
	} // ()

  public: 
	// .............................................................
	// .............................................................
	// direct access to the socket: caution !
	SocketType & getSocket () {
	  return theSocket;
	} // ()

	// .............................................................
	// .............................................................
	zmq::context_t & getContext () {
	  return theContext;
	}

  }; // class

}; // namespace

#endif
