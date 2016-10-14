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
#include <condition_variable>

// -----------------------------------------------------------------
// -----------------------------------------------------------------
namespace zmqHelper {

  // ---------------------------------------------------------------
  /// useful type declarations
  // ---------------------------------------------------------------
  using ZmqSocketType = zmq::socket_t;

  // -----------------------------------------------------------------
  /// @return true if there is more incoming data in the socket
  /// (i.e. it is a multipart message).
  // -----------------------------------------------------------------
  bool hasMore (ZmqSocketType * socket) {
	int64_t more = 0;           //  Multipart detection
	size_t more_size = sizeof (more);
	socket->getsockopt(ZMQ_RCVMORE, &more, &more_size);
  
	return more;
  } // ()

  // -----------------------------------------------------------------
  /// @return true if there is data wating to be received in the socket
  /// The thread blocks for 200ms by default.
  // -----------------------------------------------------------------
  bool isDataWaiting (ZmqSocketType * socket, long time = 200) {
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
  // ---------------------------------------------------------------
  class SocketOwnedByOtherThreadException {};
  class ThreadIsNotIddleException {};
  class CantSendDataException {};

  // ---------------------------------------------------------------
  // ---------------------------------------------------------------
  /// 
  /// The SocketAdaptor class: It wraps a zmq::socket_t.
  /// 
  // ---------------------------------------------------------------
  // ---------------------------------------------------------------
  template<int ZMQ_SOCKET_TYPE>
	class SocketAdaptor {

  private:
  
	// .............................................................
	/// 
	std::thread::id ownerThreadId;

	// .............................................................
	/// The zmq::socket_t and its context
	// .............................................................

	// Caution: member fields are initialized in this order
	// -- as they appear defined here (not as they are listed in the constructor)
	// Because theZmqSocket (and maybe theContext)
	// depends on defaultContext: defaultContext MUST appear BEFORE them
	// http://stackoverflow.com/questions/6308915/member-fields-order-of-construction

	zmq::context_t defaultContext{1}; // constructed now FIRST THIS ONE
	zmq::context_t & theContext; // not initialized now

	ZmqSocketType  theZmqSocket; // not constructed now

	// .............................................................
	/// 
	// .............................................................
	inline void checkThreadIdentity () {
	  
	  if (ownerThreadId !=  std::this_thread::get_id()) {
		std::cerr << " > > > checkThreadId FAILED \n";
		std::cerr << "ownerThreadId = " << ownerThreadId << "\n";
		std::cerr << "offending thread = " << std::this_thread::get_id() << "\n";
		throw SocketOwnedByOtherThreadException {};
	  }

	}

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
	explicit SocketAdaptor () 
	  : SocketAdaptor { defaultContext } // forward constructor
	{
	}
	  
	// .............................................................
	/// Constructor with a specific context. 
	/// Necessary when using the 'inproc' transport, as the sockets
	/// must share a zmq::context_t.
	/// @param aContext the context we get.
	// .............................................................
	explicit SocketAdaptor (zmq::context_t & aContext) 
	  :
	  theContext {aContext}, // initialized now
	  theZmqSocket {aContext, ZMQ_SOCKET_TYPE} // constructed now
	{ 
	  // std::cerr << " < < < < < < SocketAdaptor constructor starting \n" << std::flush;

	  // We catch the calling thread id just to
	  // ensure that this thread is the only using this socket.
	  // (this will be checked by checkThreadId)
	  
	  ownerThreadId = std::this_thread::get_id();

	  // std::cerr << " < < < < < < SocketAdaptor constructuctor done \n" << std::flush;
	}

	// .............................................................
	/// Destructor. Clean up.
	// .............................................................
	~SocketAdaptor ()  { 
	  // std::cerr << " > > > > > zmqHelper.destructor() called \n";
	  if ( ownerThreadId == std::this_thread::get_id() ) {
		// std::cerr << " > > > > > zmqHelper.destructor() calling close \n";
		close();
	  } 
	}

	// .............................................................
	/// bind to an url
	// .............................................................
	void bind (const std::string & url)  { 
	  checkThreadIdentity (); 

	  theZmqSocket.bind (url.c_str());
	}

	// .............................................................
	/// connect to an url
	// .............................................................
	void connect (const std::string & url)  { 
	  // std::cerr << " > > > > > SocketAdaptor.connect called \n";
	  checkThreadIdentity (); 

	  theZmqSocket.connect (url.c_str());
	  // std::cerr << " > > > > > SocketAdaptor.connect done \n";
	}

	// .............................................................
	/// disconnect from the url
	// .............................................................
	void disconnect (const std::string & url)  { 
	  checkThreadIdentity (); 

	  theZmqSocket.disconnect (url.c_str());
	}

	// .............................................................
	/// connected?
	// .............................................................
	bool isConnected () {
	  checkThreadIdentity (); 

	  return theZmqSocket.connected();
	}

	// .............................................................
	/// subscribe (pub-sub patter,  ZMQ_SUB sockets)
	// .............................................................
	void subscribe (const std::string & filter)  { 
	  checkThreadIdentity (); 

	  theZmqSocket.setsockopt(ZMQ_SUBSCRIBE, filter.c_str(), filter.size());
	}

	// .............................................................
	/// Send a multipart text message
	/// @param msgs The lines of text to send out.
	/// Call canSendData() to make sure we can send.
	/// (Don't know it is a good idea to use an assert() for it):
	// .............................................................
	void sendText (const std::vector<std::string> & msgs) {

	  checkThreadIdentity (); 
	  
	  // assert ( canSendData (theZmqSocket) );
	  if ( ! canSendData (&theZmqSocket) ) throw CantSendDataException {};

	  unsigned int many = msgs.size ();
	  unsigned int i=1;
	  for (auto msg : msgs) {
		zmq::message_t reply (msg.size());
		memcpy ((void *) reply.data (), msg.c_str(), msg.size());
		
		int more = i<many ? ZMQ_SNDMORE : 0;
		// std::cerr << " sending part " << i << "more = " << more << "\n";
		theZmqSocket.send (reply, more);
		i++;
	  }

	} // ()

	// .............................................................
	/// Receive a multipart (also a single part) text message. (blocking)
	// .............................................................
	bool receiveText (std::vector<std::string> & out) {
	  // std::cerr << " \t\t\t\t\t\t\t receiveText called \n";
	  return receiveTextInTimeout (out, -1);
	}

	// .............................................................
	/// Receive a  multipart with timeout
	// .............................................................
	bool receiveTextInTimeout (std::vector<std::string> & out, long time) {

	  checkThreadIdentity (); 

	  out.clear ();

	  if (! isDataWaiting (& theZmqSocket, time)) {
		return false;
	  }
		
	  do {
		zmq::message_t reply;
		theZmqSocket.recv (&reply); // this blocks as well, but it makes the program to abort
		// if a different thread closes the socket
		
		// char buff[100];
		// memcpy (buff, reply.data(), reply.size());
		out.push_back ( std::string { (char*) reply.data(), reply.size() } );
		
	  } while ( hasMore( & theZmqSocket ) );

	  return true;
	
	} // ()

	// .............................................................
	/// Close the socket 
	// .............................................................
	const void close () {

	  // destructor-thread calls close() only
	  // if owner of the socket
	  checkThreadIdentity (); 

	  //
	  // close 
	  //
   	  theZmqSocket.close (); 
	  
	  // std::cerr << " > > > > > SocketAdaptor.close(): zmq socket closed \n";

	} // ()

	// .............................................................
	/// Direct access to the wrapped socket if a function
	/// not covered here is needed.
	/// 
	///    Warning: 
	/// now the class loses control over the socket
	/// and thread-safety (only one thread uses the socket "at a time")
	/// can't be guaranteed. Use on your own risk.
	/// @return Reference to the socket.
	// .............................................................
	ZmqSocketType  * getZmqSocket () {
	  checkThreadIdentity (); 
	  return & theZmqSocket;
	} // ()

	// .............................................................
	/// Direct access to the context of this socket for if 
	/// a function not covered here is needed.
	/// zmq_context are thread-safe, but not zmq_sockets.
	/// 
	/// You create  the zmq_socket and pass the context to it.
	/// But, it seems that the internal zmq_socket thread
	/// is in charge of saving the context. If
	/// this latter thread gets blocked, and a different
	/// one asks for the context before it is properly set,
	/// a wrong context is returned
	/// or even the operation fails.
	///  
	/// Hence, we check the identity of the caller asking for the context.
	/// 
	/// If you need to shared a context create it independently
	/// of the sockets and share it, not ask the stored context
	/// for sharing.
	/// 
	/// @return Reference to the context.
	// .............................................................
	// .............................................................
	zmq::context_t & getContext () {
	  // std::cerr << " > > > > > SocketAdaptor.getContext () called \n";
	  checkThreadIdentity (); 
	  return theContext;
	}

  }; // class










  
  // ---------------------------------------------------------------
  // ---------------------------------------------------------------
  /// 
  /// The SocketAdaptorWithThread class: 
  /// 
  // ---------------------------------------------------------------
  // ---------------------------------------------------------------
  template<int ZMQ_SOCKET_TYPE>
  class SocketAdaptorWithThread {

  public:

	using SocketAdaptorType = SocketAdaptor<ZMQ_SOCKET_TYPE>;
	using FunctionType = std::function<void(SocketAdaptorType&)>;

  private:

	// .............................................................
	/// 
	SocketAdaptorType * theSocketAdaptor = nullptr;

	// Caution: member fields are initialized in this order
	// -- as they appear defined here (not as they are listed in the constructor)
	// Because theContext may
	// depend on defaultContext: defaultContext MUST appear BEFORE it
	// http://stackoverflow.com/questions/6308915/member-fields-order-of-construction
	zmq::context_t defaultContext {1}; // constructed now FIRST THIS ONE
	zmq::context_t & theContext;  // not init now
  
	// .............................................................
	/// Funcion to call if a internal thread is used
	FunctionType theCallback = nullptr;

	// .............................................................
	/// Thread handler (when a dedicated thread is used USED)
	/// the SocketAdaptor.
	std::thread * theThread = nullptr;
	bool threadRunning = false;

	// .............................................................
	///
	std::mutex theMutex; // to be shared by locks on the same "subject"
	std::condition_variable conditionVar; // to be shared ...
	using Lock = std::unique_lock<std::mutex>;

	// .............................................................
	// .............................................................
	void createTheThread () {

	  theCallback = nullptr;

	  // 
	  // This is where the inner thread is created.
	  // Its first action is to create the socket.
	  // Then it calls main_Thread, where
	  // the assigned task will be run.
	  // 
	  theThread = new std::thread ( [this] () { 
		  //
		  //
		  //
		  theSocketAdaptor = new SocketAdaptor<ZMQ_SOCKET_TYPE> {theContext};
		  //
		  //
		  //
		  main_Thread(); 

		  // std::cerr << " > > > > > createTheThread() LAMBDA: after main_Thread()\n";

		  //
		  //
		  //
		  closeSocketAdaptor ();
		  // std::cerr << " > > > > > createTheThread() LAMBDA: at end\n";
		} );

	  // std::cerr << " > > > > > createTheThread() done \n";

	} // ()

	// .............................................................
	// .............................................................
	void awakeTheThread () {

	  // std::cerr << " > > > > > zmqHelper.awakeTheThread() called \n";

	  assert ( theCallback != nullptr );
	  assert ( theThread != nullptr );
	  
	  threadRunning = true;
	  conditionVar.notify_one();

	  // std::cerr << " > > > > > awakeTheThread() done \n";
	} // ()

	// .............................................................
	/// 
	// .............................................................
	  void assignTaskToTheThread ( FunctionType f)
	{ 

	  if (! isThreadIdle () ) {
		throw ThreadIsNotIddleException {};
		// return;
	  }

	  //
	  //
	  //
	  theCallback = f;

	  //
	  //
	  //
	  awakeTheThread ();

	  // std::cerr << " > > > > > assignThasToTheThread() done \n";
	}

	// .............................................................
	/// 
	// .............................................................
	void closeSocketAdaptor () {
	  if ( theSocketAdaptor == nullptr) {
		return;
	  }
	  //
	  // close the socket and clean up
	  //
	  theSocketAdaptor->close ();                                           
	  delete theSocketAdaptor;
	  theSocketAdaptor = nullptr;

	  // std::cerr << " > > > > > closeSocketAdaptor() done \n";
	}


	// .............................................................
	// .............................................................
	void main_Thread () {

	  // std::cerr << " > > > > > zmqHelper.mainThread() called \n";

	  Lock theLock {theMutex};

	  threadRunning = true;

	  // 
	  // 
	  // 
	  while (threadRunning) {

		while ( theCallback == nullptr) {
		  // while: just to make sure we try to block (wait)
		  // if there is no task to do
		   // std::cerr << " > > > > > zmqHelper.mainThread(): waiting  \n";
		   if ( ! threadRunning ) break;
		   try {
			 conditionVar.wait (theLock) ; // wait 
		   } catch (std::exception err) {
			 // std::cerr << " > > > > > zmqHelper.mainThread(): wait EXCEPTION  \n";
			 threadRunning = false;
		   }
		}

		 // std::cerr << " > > > > > zmqHelper.mainThread(): unlocked  \n";

		if ( ! threadRunning ) break;
	  
		if (theCallback  != nullptr ) {
		  // do it once
		  theCallback (*theSocketAdaptor);
		  // and because, for the moment no other task
		  // is going to be assigned to this thread. Finish:
		  threadRunning = false;
		}

	  } // while

	  theCallback = nullptr;

	  // std::cerr << " > > > > > zmqHelper.mainThread(): end of life  \n";

	} // ()


	// .............................................................
	/// Copy construction disallowed.
	// .............................................................
	SocketAdaptorWithThread (const SocketAdaptorWithThread & o)  = delete;

	// .............................................................
	/// Assignment disallowed.
	// .............................................................
	SocketAdaptorWithThread & operator=(const SocketAdaptorWithThread & o)  =delete;

	// .............................................................
	/// no default constructor
	// .............................................................
	SocketAdaptorWithThread () = delete;

  public:

	// .............................................................
	/// Constructor 
	/// @param aContext the context to use.
	/// @param f the task for the inner thread
	// .............................................................
	explicit SocketAdaptorWithThread (zmq::context_t & aContext, FunctionType  f)
	  : 
		theContext{aContext} // init now
	{ 
	  createTheThread ();
	  assignTaskToTheThread ( f );
	  // std::cerr << " > > > > > zmqHelper.constructor() with inner thread ended \n";
	}

	// .............................................................
	/// Constructor.  Use own context.
	/// @param f the task for the inner thread
	// .............................................................
	explicit SocketAdaptorWithThread (FunctionType  f)
	  : SocketAdaptorWithThread {defaultContext, f} // forward constructor
	{
	}

	// .............................................................
	/// Destructor. Clean up.
	// .............................................................
	~SocketAdaptorWithThread ()  { 

	  // This will be run by a different thread
	  // from the one created here. That's why we
	  // don't try to close the socket.
	  // We only try to stop and join the thread

	  // (But this does not guarantee that the thread is going
	  // to be stopped, since it can be, f.ex., blocked
	  // in a receive).
	  // 
	  tryToStopAndJoinTheThread ();
	}

	// .............................................................
	/// The calling thread waits to join the inner thread
	// .............................................................
	void joinTheThread () {

	  // std::cerr << " > > > > > joinTheThread(): called \n";

	  if ( theThread == nullptr ) {
		// no running thread: nothing to do
		return;
	  }

	  if ( ! theThread->joinable() ) {
		// the thread can't be joined: nothing to do
		return;
	  }

	  try {

		// std::cerr << " > > > > > zmqHelper.joinTheThread(): going to join\n";

		//
		// join and clean up
		//
		theThread->join ();

		// std::cerr << " > > > > > zmqHelper.joinTheThread(): joined \n";
		
		delete theThread;
		theThread = nullptr;
		
	  } catch ( std::exception ex) {
		// std::cerr << " > > > > > zmqHelper.joinTheThread(): exception in ->join" << ex.what() << "\n";
		// std::cerr << " > > > > > this thread = " << std::this_thread::get_id() << "\n";
		return;
	  }
		
	  	  // std::cerr << " > > > > > zmqHelper.joinTheThread(): done \n";

	} // ()

	// .............................................................
	/// Allow the thread to exit the loop in main_Thread.
	/// (Anyway the thread can be blocked, f.ex, receiving.
	/// Thus, the programmer should provide a way to unblock
	/// a inner thread if he uses it)
	// .............................................................
	void tryToStopAndJoinTheThread () {

	  // std::cerr << " > > > > > trytoStopAndJoinTheThread () called \n" << std::flush;

	  if ( theThread == nullptr ) {
		// no running thread: nothing to do
		return;
	  }

	  //
	  // stop
	  //
	  threadRunning = false;
	  conditionVar.notify_one();

	  //
	  // join 
	  //
	  joinTheThread ();

	} // ()

	// .............................................................
	/// is the thread idle?
	// .............................................................
	bool isThreadIdle () {
	  return (theThread != nullptr && theCallback == nullptr);
	}

	// .............................................................
	/// Direct access to the wrapped socket if a function
	/// not covered here is needed.
	/// 
	///    Warning: 
	/// now the class loses control over the socket
	/// and thread-safety (only one thread uses the socket "at a time")
	/// can't be guaranteed. Use on your own risk.
	/// @return Reference to the socket.
	// .............................................................
	ZmqSocketType * getZmqSocket () {
	  return theSocketAdaptor->getZmqSocket ();
	} // ()

  private:
	
	// .............................................................
	/// 
	/// Function removed. See comment of SocketAdaptor::getContext()
	/// Here, in addition, this function can only be called
	/// by an external thread, different from the inner (and owner) one.
	/// So it should not even exist.
	/// 
	/// @return Reference to the context.
	// .............................................................
	/*
	zmq::context_t & getContext () {
	  // std::cerr << " > > > > > SocketAdaptorWithThread.getContext () called \n";
	  return theSocketAdaptor->getContext ();
	}
	*/
	
  }; // class

}; // namespace

#endif
