libvci
======

A C library with implementations for basic everyday stuff.

FEATURES
--------

( TODO )

### concurrent

### container

### util

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
