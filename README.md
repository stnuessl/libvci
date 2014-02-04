libvci
======

A C library with implementations for basic everyday stuff.

FEATURES
--------

### clock
Clock implementation for messuring elapsed time.

### config
Config interface for reading out config files.

### container
Implementation of various container types. Includes:

        AVL Tree                --> struct avltree
        Serial Buffer           --> struct buffer
        LRU Cache               --> struct cache
        List                    --> struct link
        Hashtable / Hashmap     --> struct map
        FIFO Queue              --> struct queue
        Ring-/ Circular Buffer  --> struct ringbuffer
        LIFO Stack              --> struct stack
        Dynamic Array           --> struct vector

### etrace
Interface for stacktracing on critical errors.

### mempool
Mempool implementation for fast memory allocation on fixed memory sizes.

### threadpool
Threadpool implementation for managing tasks and threads.

        Task:           Defining callback function and arguments.
        --> struct task
        
        Threadpool:     Executes inserted tasks in FIFO Order.
        --> struct threadpool

### observer
Register a list of callbacks that get executed on defined events.

        Observer:       Define event and callback which shall get executed.
        --> struct observer

        Subject:        Register observer and signal events.
        --> struct subject
        
### log
Logging functions providing configurable line header and severity levels.


INSTALLING
----------

Library needs Linux version >= 2.6.22 (eventfd) and glibc version >= 2.9 
and is tested to be compilable with gcc 4.x.x.

	$ git clone git://github.com/stnuessl/libvci
	$ mkdir libvci/build
	$ cd libvci/build
	$ cmake ../
	$ make
	$ make install

Adjust the CMakeLists.txt to install to a different directory.

To install the test (example) applications, do:

	$ mkdir ../src/tests/build
	$ cd ../src/tests/build
	$ cmake ../
	$ make

DOCUMENTATION
-------------

None, because I don't need one. If you really like to use this library and 
want/need some documentation, leave me a message.
