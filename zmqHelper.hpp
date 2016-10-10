/*
 * -----------------------------------------------------------------
 * zmqHelper.hpp
 *
 * SocketAdaptor class to help using ZeroMQ sockets. 
 * Features C++11
 * Based on zmq.hpp
 *
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

// -----------------------------------------------------------------
// -----------------------------------------------------------------
namespace zmqHelper {

  // ---------------------------------------------------------------
  /// forward declaration
  // ---------------------------------------------------------------
  template<int ZMQ_SOCKET_TYPE> class SocketAdaptor; 

  // ---------------------------------------------------------------
  /// useful type declarations
  // ---------------------------------------------------------------
  using ZmqSocketType = zmq::socket_t;

  template<int ZMQ_SOCKET_TYPE>
  using FunctionType = std::function<void(SocketAdaptor<ZMQ_SOCKET_TYPE>&)>;

  // -----------------------------------------------------------------
  /// @return true if there is more incoming data in the socket
  /// (i.e. it is a multipart message).
  // -----------------------------------------------------------------
  template<typename SocketType> bool hasMore (SocketType * socket) {
	int64_t more = 0;           //  Multipart detection
	size_t more_size = sizeof (more);
	socket->getsockopt(ZMQ_RCVMORE, &more, &more_size);
  
	return more;
  } // ()

  // -----------------------------------------------------------------
  /// @return true if there is data wating to be received in the socket
  /// The thread blocks for 200ms by default.
  // -----------------------------------------------------------------
  template<typename SocketType> bool isDataWaiting (SocketType * socket, long time = 200) {
	// std::cerr << " >>> \t\t\t\t\t\t\t isDataWaiting() time = " << time << "\n";
	try {
	  zmq::pollitem_t items [] = { { *socket, 0, ZMQ_POLLIN, 0} };
	  int some = zmq::poll ( &items[0], 1, time); 
	  // timeout=200ms 
	  // some>0 => something arrived
	  // zmq::poll ( &items[0], 1, -1); // -1 = blocking
	  
	  return some>0;
	} catch ( std::exception ex) {
	  // std::cerr << " isDataWaiting EXCEPTION \n";
	  return false;
	}
  } // ()

  // -----------------------------------------------------------------
  /// Wait (blocked) for incoming data on any of the listed sockets.
  /// (isDataWaiting() version for n-sockets, blocked)
  /// @return the pointer of the zmq socket for which data is available.
  // -----------------------------------------------------------------
  ZmqSocketType * waitForDataInSockets (const std::vector<ZmqSocketType *> & list) {
	try {
	  zmq::pollitem_t items [list.size()]; 
	  for (unsigned int i=0; i<=list.size()-1; i++) {
		items[i] = { *list[i], 0, ZMQ_POLLIN, 0};
	  }
	  
	  zmq::poll ( &items[0], list.size(), -1);
	  
	  for (unsigned int i=0; i<=list.size()-1; i++) {
		if ( items[i].revents & ZMQ_POLLIN ) {
		  return list[i];
		}
	  }

	  return nullptr;

	} catch ( std::exception ex) {
	  // std::cerr << " waitForDataInSockets EXCEPTION \n";
	  
	  return nullptr;
	}
	
  } // ()

  // -----------------------------------------------------------------
  /// @return true if data can be sent
  /// Note: the thread is blocked in zmq::poll()
  // -----------------------------------------------------------------
  template<typename SocketType> bool canSendData (SocketType * socket) {
	try {
	  zmq::pollitem_t items [] = { { (*socket), 0, ZMQ_POLLOUT, 0} };
	  int some = zmq::poll ( &items[0], 1, -1); 
	  // timeout=200ms 
	  // some>0 => something can be sent
	  // zmq::poll ( &items[0], 1, -1); // -1 = blocking
	  
	  return some>0;
	  
	} catch ( std::exception ex) {
	  // std::cerr << " canSendData EXCEPTION \n";
	  return false;
	}
  } // ()


  // ---------------------------------------------------------------
  /// 
  /// The SocketAdaptor class. 
  /// It wraps a zmq::socket_t.
  /// 
  // ---------------------------------------------------------------
  template<int ZMQ_SOCKET_TYPE>
	class SocketAdaptor {

  private:
  
	// .............................................................
	/// Funcion to call when data arrives.
	FunctionType<ZMQ_SOCKET_TYPE> theCallback;
	/// Is the callback valid?
	bool callbackValid = false;

	// .............................................................
	/// Thread handler (when a dedicated thread is used to read in 
	/// data).
	std::thread * theThread = nullptr;

	// .............................................................
	// Trying to protect two threads sending/receiving at the same
	// THIS CAN'T HAPPEN. FORBIDEN BY 0MQ: TWO THREADS SHOULDN'T
	// SHARE A SOCKET
	/// time.
	// std::mutex theMutex;
	// std::unique_lock<std::mutex> theLock {theMutex, std::defer_lock}; 
	// #define LOCK std::unique_lock<std::mutex> theLock {theMutex};
	// #define LOCK 
	// 
 
	// .............................................................
	/// default context for the zmq::socket_t
	zmq::context_t defaultContext {1};

	// .............................................................
	/// The zmq::socket_t and its context
	zmq::context_t & theContext; 
	ZmqSocketType  * theZmqSocket = nullptr;

	// .............................................................
	/// Function where the socket is polled to know if data has 
	/// in arrived. If so, the callback is called.
	/// @see isDataWaiting()
	/// 
	/// CAUTION: no other threads may access this socket
	/// so, let's block
	/// 
	///  Previous version comment:
	/// (Not sure about the 200ms timeout setting for zmq::poll.
	///   I think we should not block the thread on the poll (-1)
	///  to allow other threads send).
	// .............................................................
	void main () {

	  // std::cerr << " main starts \n";

	  while ( callbackValid ) {
	  
		// 
		// if data has arrived, call the handler
		// isDataWaiting() blocks: -1 param
		// 
		if ( isDataWaiting(theZmqSocket, -1) && callbackValid ) {
		  theCallback (*this);
		}

	  } // while

	  // std::cerr << " main ends \n";
	} // ()

	// .............................................................
	/// Copy construction disallowed.
	// .............................................................
	SocketAdaptor (const SocketAdaptor & o)  = delete;

	// .............................................................
	/// Assignment disallowed.
	// .............................................................
	SocketAdaptor & operator=(const SocketAdaptor & o)  = delete;

  public:

	// .............................................................
	/// Default constructor. (Use our own zmq::context_t).
	// .............................................................
	SocketAdaptor () 
	  : theContext {defaultContext}, theZmqSocket { new ZmqSocketType {theContext, ZMQ_SOCKET_TYPE} }
	  { }

	// .............................................................
	/// Constructor with a specific context. 
	/// Necessary when using the 'inproc' transport, as the sockets
	/// must share a zmq::context_t.
	/// @param aContext the context we get.
	// .............................................................
	SocketAdaptor (zmq::context_t & aContext) 
	  : theContext{aContext}, theZmqSocket { new ZmqSocketType {aContext, ZMQ_SOCKET_TYPE} }
	{ }

	// .............................................................
	/// Destructor. Clean up.
	// .............................................................
	~SocketAdaptor ()  { 
	  // std::cerr << " > > > > > zmqHelper.destructor() called \n";
	  close();
	}

	// .............................................................
	/// bind to an url
	// .............................................................
	void bind (const std::string & url)  { 
	  theZmqSocket->bind (url.c_str());
	}

	// .............................................................
	/// connect to an url
	// .............................................................
	void connect (const std::string & url)  { 
	  theZmqSocket->connect (url.c_str());
	}

	// .............................................................
	/// disconnect from the url
	// .............................................................
	void disconnect (const std::string & url)  { 
	  theZmqSocket->disconnect (url.c_str());
	}

	// .............................................................
	/// connected?
	// .............................................................
	bool isConnected () {
	  return theZmqSocket->connected();
	}

	// .............................................................
	/// subscribe (pub-sub patter,  ZMQ_SUB sockets)
	// .............................................................
	void subscribe (const std::string & filter)  { 
	  theZmqSocket->setsockopt(ZMQ_SUBSCRIBE, filter.c_str(), filter.size());
	}

	// .............................................................
	/// Install f as the function to call when data arrives in.
	/// @param startThread By default a dedicated thread is created
	/// to wait for incoming data.
	/// 
	/// Caution with this. 
	/// 
	/// ZMQ discourages that two threads sharing the same socket.
	/// Even the case that one is only receiving (i.e. code installed with onMessage())
	/// an a diferent one is sending.
	/// 
	/// 
	/// Polling is the recomended way to go
	/// 
	// .............................................................
	void onMessage (FunctionType<ZMQ_SOCKET_TYPE>  f, bool startThread=true) {
   
	  if (callbackValid) {
		// running, must first call stop_receiving()
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
	/// The calling thread itself waits for incoming data.
	// .............................................................
	void receiveMessages (FunctionType<ZMQ_SOCKET_TYPE>  f) {
	  onMessage (f, false);
	} // ()

	// .............................................................
	// .............................................................
	void unsetCallbackValid () {
	  if (! callbackValid) {
		// not running, nothing to do
		return;
	  }

	  // std::cerr << " stopping thread now \n";

	  // this stops the thread loop
	  callbackValid = false;
	}


	// .............................................................
	/// Uninstall the callback (the loop on main() ends, and
	/// its thread is joined.)
	// .............................................................
	void stopReceiving () {
	  unsetCallbackValid ();
	  // Warning: if the thread is blocked
	  // we are going to be blocked here as well.
	  joinTheThread ();
	}

	// .............................................................
	/// Join the thread in main() (if any) 
	// .............................................................
	void joinTheThread () {

	  if ( theThread == nullptr ) {
		// no running thread: nothing to do
		return;
	  }

	  theThread->join ();
	  delete (theThread);
	  theThread = nullptr;

	} // ()

	// .............................................................
	/// Send a multipart text message
	/// @param msgs The lines of text to send out.
	/// Call canSendData() to make sure we can send.
	/// (Don't know it is a good idea to use an assert() for it):
	// .............................................................
	void sendText (const std::vector<std::string> & msgs) {
	  
	  // is locking a good idea? No
	  // LOCK; // theLock.lock();

	  // assert ( canSendData (theZmqSocket) );
	  if ( ! canSendData (theZmqSocket) ) throw zmq::error_t();

	  unsigned int many = msgs.size ();
	  unsigned int i=1;
	  for (auto msg : msgs) {
		zmq::message_t reply (msg.size());
		memcpy ((void *) reply.data (), msg.c_str(), msg.size());
		
		int more = i<many ? ZMQ_SNDMORE : 0;
		// std::cerr << " sending part " << i << "more = " << more << "\n";
		theZmqSocket->send (reply, more);
		i++;
	  }

	  // theLock.unlock();
	} // ()

	// .............................................................
	/// Send a single line message.
	/// @param msg The text to send
	/// Call canSendData() to make sure we can send.
	/// (Don't know it is a good idea to use an assert() for it):
	// .............................................................
	void sendText (const std::string & msg) {

	  // is locking a good idea? NO
	  // LOCK; // theLock.lock();

	  // assert ( canSendData (theZmqSocket) );
	  if ( ! canSendData (theZmqSocket) ) throw zmq::error_t();

	  zmq::message_t reply (msg.size());
	  memcpy ((void *) reply.data (), msg.c_str(), msg.size());
	  theZmqSocket->send (reply);

	  // theLock.unlock();
	} // ()

	// .............................................................
	/// Receive a multipart (also a single part) text message. (blocking)
	// .............................................................
	const std::vector<std::string> receiveText () {

	  // is locking a good idea? no
	  // LOCK; // theLock.lock();

	  std::vector<std::string> result;

	  do {
		zmq::message_t reply;
		theZmqSocket->recv (&reply);

		// char buff[100];
		// memcpy (buff, reply.data(), reply.size());
		result.push_back ( std::string { (char*) reply.data(), reply.size() } );
		
	  } while ( hasMore( theZmqSocket ) );
	  
	  // theLock.unlock();

	  return result;
	  
	} // ()

	// .............................................................
	/// Receive, multipart with timeout
	// .............................................................
	bool receiveTextInTimeout (std::vector<std::string> & out, long time) {
	  if (isDataWaiting (theZmqSocket, time)) {
		out = receiveText ();
		return true;
	  }

	  return false;
	} // ()

	// .............................................................
	/// Close the socket.
	// .............................................................
	const void close () {
	  if ( theZmqSocket == nullptr) {
		return;
	  }

	  unsetCallbackValid ();
   	  theZmqSocket->close (); /* close before stop receving
							  * because if the thread is blocked
							  * (receiving for instance)
							  * the join() in stop_Receiving()
							  * will block us here as well
							  */

	  delete theZmqSocket;
	  theZmqSocket = nullptr;

	  stopReceiving (); 

	} // ()

  public: 
	// .............................................................
	/// Direct access to the wrapped socket if a function
	/// not covered here is needed.
	/// 
	///    Warning: 
	/// now the class loses control over the socket
	/// and thread-safety (only one thread uses the socket "at a time")
	/// can't be guaranteed
	/// @return Reference to the socket.
	// .............................................................
	ZmqSocketType * getZmqSocket () {
	  return theZmqSocket;
	} // ()

	// .............................................................
	/// Direct access to the context of this socket for if 
	/// a function not covered here is needed.
	/// @return Reference to the context.
	// .............................................................
	zmq::context_t & getContext () {
	  return theContext;
	}

  }; // class

}; // namespace

#endif
