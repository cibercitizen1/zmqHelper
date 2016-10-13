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
	- SocketAdaptorWithThread runs a new thread witch exclusive use to the enclosed SocketAdaptor. The constructor expects a function that will be executed by the inner thread.

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

```cpp
```

```cpp
```
