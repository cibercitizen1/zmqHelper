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

* If you use onMessage() to install a callback, you can choose
if a new thread is launched to listen for incoming data.  In this case,
the thread using the the socket, will be different from the thread that created it.
In this case, you should allow only the thread in the callback to use the socket.


```cpp
```

