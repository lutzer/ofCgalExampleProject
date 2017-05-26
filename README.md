# ofCgalExampleProject

Example xCode project for including CGAL 4.9.1 within an open Frameworks Project

## Setting up IDE

* create Project within <openframeworks_folder>/apps/myApps
* install cgal and dependencies with homebrew: `brew install cgal`
* go to the projects build settings
  * add `/usr/local/lib` to Library Search Paths and `/usr/local/include` to Header Serach Paths
  * within *User Definded* change where openframeworks looks for boost headers:
    ```
    HEADER_BOOST = "/usr/local/include"
    LIB_BOOST_SYSTEM = "/usr/local/lib/libboost_system.a"
    LIB_BOOST_FS = "/usr/local/lib/libboost_filesystem.a"
    ```
  * go to Build Phases Tab and add the following libraries from the folder */usr/local/lib*
    * libboost_system-mt.dylib
    * libboost_thread-mt.dylib
    * libCGAL_Core.dylib
    * libCGAL.dylib
    * libgmp.dylib
    * libmpfr.dylib
  * make sure language dialect is set to c++11 (should be the default setting)
