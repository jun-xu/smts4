smts4.0
======

**smts4.0** 
use libuv.
all of smts4.0 in [jira](http://192.168.203.103:8080/browse/CSS-2).

---------



opensource libs
----------

 1. **libuv** 
    An introduction to [libuv](http://docs.libuv.org/en/latest/).

----------


gits
----------


----------


dirs
---------

 - **`$CSS_HOME/bin`**
    all binary file of smts4.0.

 - **`$CSS_HOME/deps`**
    all libs of smts4.0 dependent.

     > `$CSS_HOME/deps/lib_uv/` *all source files of lib_uv.*

 - **`$CSS_HOME/src`**
    all source file like xxx.c and xxx.h of smts4.0.

 - **`$CSS_HOME/test`**
    all unit tests of smts4.0.  

    
    
    


----------


Build Instructions
----------

####1. **linux**
first compile: **`$compile_linux_uv`**
other compile:  **`$make compile_linux`** or **`$make compile_linux_test`** for test.
all make cmd here:

 - **`$make get_deps`**         get all dependents.
 - **`$make compile_linux_uv`**       compile libuv independent.
 - **`$make compile_linux`**      compile smts4.0 source files independent without test.
 - **`$make compile_linux_test`** compile smts4.0 source files independent with test.
 - **`$make compile_all`**      compile compile smts4.0 source file without test.
 - **`$make`**                  like `$make compile_all`
 - **`$make clean`**            clean smts4.0 binary file.
 - **`$make clean_all`**        clean all smts4.0 binary and dependents's file.
   
   
####2. **OS X**
first compile: **`$compile_uv`**
other compile:  **`$make compile`** or **`$make compile_test`** for test.
all make cmd here:

 - **`$make get_deps`**         get all dependents.
 - **`$make compile_uv`**       compile libuv independent.
 - **`$make compile`**      compile smts4.0 source files independent without test.
 - **`$make compile_test`** compile smts4.0 source files independent with test.
 - **`$make compile_all`**      compile compile smts4.0 source file without test.
 - **`$make`**                  like `$make compile_all`
 - **`$make clean`**            clean smts4.0 binary file.
 - **`$make clean_all`**        clean all smts4.0 binary and dependents's file.    

Supported Platforms
---------------

Microsoft Windows operating systems since Windows XP SP2. It can be built
with either Visual Studio or MinGW. Consider using
Visual Studio Express 2010 or later if you do not have a full Visual
Studio license.

Linux using the GCC toolchain.

OS X using the GCC or XCode toolchain.

Patches
-------------

 - [node.js](http://nodejs.org/)
 - [GYP](http://code.google.com/p/gyp/)
 - [Python](https://www.python.org/downloads/)
 - [Visual Studio Express 2010]( http://www.microsoft.com/visualstudio/eng/products/visual-studio-2010-express)
 - [mxml](http://www.msweet.org/projects.php?Z3)
 - [mysql](http://www.mysql.com)
 - [uv](https://github.com/joyent/libuv)
 - [cmake](http://www.cmake.org/cmake/resources/software.html)