/*
 * -----------------------------------------------------------------
 * zmqHelper.h
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

  // -----------------------------------------------------------------
  /// @return true if there is more incoming data in the socket
  /// (i.e. it is a multipart message).
  // -----------------------------------------------------------------
  template<typename SocketType> bool hasMore (SocketType & socket) {
	int64_t more = 0;           //  Multipart detection
	size_t more_size = sizeof (more);
	socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
  
	return more;
  } // ()

  // -----------------------------------------------------------------
  /// @return true if there is data wating to be received in the socket
  /// The thread blocks for 200ms by default.
  // -----------------------------------------------------------------
  template<typename SocketType> bool isDataWaiting (SocketType & socket, long time = 200) {
	try {
	  zmq::pollitem_t items [] = { { socket, 0, ZMQ_POLLIN, 0} };
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
  /// @return true if data can be sent
  /// Note: the thread is blocked in zmq::poll()
  // -----------------------------------------------------------------
  template<typename SocketType> bool canSendData (SocketType & socket) {
		zmq::pollitem_t items [] = { { socket, 0, ZMQ_POLLOUT, 0} };
		int some = zmq::poll ( &items[0], 1, -1); 
		// timeout=200ms 
		// some>0 => something can be sent
		// zmq::poll ( &items[0], 1, -1); // -1 = blocking

		return some>0;
  } // ()


  // ---------------------------------------------------------------
  /// forward declaration
  // ---------------------------------------------------------------
  template<int SOCKET_TYPE> class SocketAdaptor; 

  // ---------------------------------------------------------------
  /// useful type declarations
  // ---------------------------------------------------------------
  using SocketType = zmq::socket_t;

  template<int SOCKET_TYPE>
  using FunctionType = std::function<void(SocketAdaptor<SOCKET_TYPE>&)>;

  // ---------------------------------------------------------------
  /// 
  /// The SocketAdaptor class. 
  /// It wraps a zmq::socket_t.
  /// 
  // ---------------------------------------------------------------
  template<int SOCKET_TYPE>
	class SocketAdaptor {

  private:
  
	// .............................................................
	/// Funcion to call when data arrives.
	FunctionType<SOCKET_TYPE> theCallback;
	/// Is the callback valid?
	bool callbackValid = false;

	// .............................................................
	/// Thread handler (when a dedicated thread is used to read in 
	/// data).
	std::thread * theThread = nullptr;

	// .............................................................
	/// Trying to protect two threads sending/receiving at the same
	/// time.
	std::mutex theMutex;
	// std::unique_lock<std::mutex> theLock {theMutex, std::defer_lock}; 
#define LOCK std::unique_lock<std::mutex> theLock {theMutex};
	// #define LOCK 
 
	// .............................................................
	/// default context for the zmq::socket_t
	zmq::context_t defaultContext {1};

	// .............................................................
	/// The zmq::socket_t and its context
	zmq::context_t & theContext; 
	SocketType  theSocket; 

	// .............................................................
	/// Function where the socket is polled to know if data has 
	/// in arrived. If so, the callback is called.
	/// @see isDataWaiting()
	///  (Not sure about the 200ms timeout setting for zmq::poll.
	///   I think we should not block the thread on the poll (-1)
	///  to allow other threads send).
	// .............................................................
	void main () {

	  // std::cerr << " main starts \n";

	  while ( callbackValid ) {
	  
		// 
		// if data has arrived, call the handler
		// isDataWaiting() blocks for 200ms, then returns.
		// 
		if ( isDataWaiting(theSocket) && callbackValid ) {
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
	  : theContext {defaultContext}, theSocket {theContext, SOCKET_TYPE}
	  { }

	// .............................................................
	/// Constructor with a specific context. 
	/// Necessary when using the 'inproc' transport, as the sockets
	/// must share a zmq::context_t.
	/// @param aContext the context we get.
	// .............................................................
	SocketAdaptor (zmq::context_t & aContext) 
	  : theContext{aContext}, theSocket {aContext, SOCKET_TYPE}
	{ }

	// .............................................................
	/// Destructor. Clean up.
	// .............................................................
	~SocketAdaptor ()  { 
	  // std::cerr << " destructor \n";
	  stopReceiving (); 
	  close();
	}

	// .............................................................
	/// bind to an url
	// .............................................................
	void bind (const std::string & url)  { 
	  theSocket.bind (url.c_str());
	}

	// .............................................................
	/// connect to an url
	// .............................................................
	void connect (const std::string & url)  { 
	  theSocket.connect (url.c_str());
	}

	// .............................................................
	/// disconnect from the url
	// .............................................................
	void disconnect (const std::string & url)  { 
	  theSocket.disconnect (url.c_str());
	}

	// .............................................................
	/// subscribe (pub-sub patter,  ZMQ_SUB sockets)
	// .............................................................
	void subscribe (const std::string & filter)  { 
	  theSocket.setsockopt(ZMQ_SUBSCRIBE, filter.c_str(), filter.size());
	}

	// .............................................................
	/// Install f as the function to call when data arrives in.
	/// @param startThread By default a dedicated thread is created
	/// to wait for incoming data.
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
	/// The calling thread itself waits for incoming data.
	// .............................................................
	void receiveMessages (FunctionType<SOCKET_TYPE>  f) {
	  onMessage (f, false);
	} // ()

	// .............................................................
	/// Uninstall the callback (the loop on main() ends, and
	/// its thread is joined.)
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
	/// Join the thread in main() (if any) 
	// .............................................................
	void wait () {
	  if ( theThread == nullptr ) {
		// no running thread: nothing to do
		return;
	  }

	  // std::cerr << " waiting thread \n";

	  theThread->join ();
	  delete (theThread);
	  theThread = nullptr;

	  // std::cerr << " waiting thread DONE \n";
	} // ()

	// .............................................................
	/// Send a multipart text message
	/// @param msgs The lines of text to send out.
	/// Call canSendData() to make sure we can send.
	/// (Don't know it is a good idea to use an assert() for it):
	// .............................................................
	void sendText (const std::vector<std::string> & msgs) {
	  
	  // is locking a good idea?
	  LOCK; // theLock.lock();

	  // assert ( canSendData (theSocket) );
	  if ( ! canSendData (theSocket) ) throw zmq::error_t();

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
	/// Send a single line message.
	/// @param msg The text to send
	/// Call canSendData() to make sure we can send.
	/// (Don't know it is a good idea to use an assert() for it):
	// .............................................................
	void sendText (const std::string & msg) {

	  // is locking a good idea?
	  LOCK; // theLock.lock();

	  // assert ( canSendData (theSocket) );
	  if ( ! canSendData (theSocket) ) throw zmq::error_t();

	  zmq::message_t reply (msg.size());
	  memcpy ((void *) reply.data (), msg.c_str(), msg.size());
	  theSocket.send (reply);

	  // theLock.unlock();
	} // ()

	// .............................................................
	/// Receive a multipart (also a single part) text message.
	// .............................................................
	const std::vector<std::string> receiveText () {

	  // is locking a good idea?
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
	/// Close the socket.
	// .............................................................
	const void close () {
	  theSocket.close ();
	} // ()

  public: 
	// .............................................................
	/// Direct access to the wrapped socket if a function
	/// not covered here is needed.
	/// @return Reference to the socket.
	// .............................................................
	SocketType & getSocket () {
	  return theSocket;
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

// http://www.stack.nl/~dimitri/doxygen/manual/docblocks.html
