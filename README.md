libvci - Library for Various C Interfaces
=========================================

A library with various interfaces for common tasks.

FEATURES
--------

### clock
Timer interface for messuring elapsed time.

### config
Config interface for reading out config files.

### container
Interface for various container types.

### etrace
Interface for stacktracing on critical errors.

### mempool
Mempool interface for fast memory allocation on fixed memory sizes.

### threadpool
Threadpool interface for managing tasks and threads.

### log
Logging functions providing configurable line header and severity levels.


INSTALLING
----------

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