# ofCgalExampleProject

Example xCode project for including CGAL 4.9.1 within an openFrameworks Project.



## Compile Project

* install cgal and dependencies with homebrew: `brew install cgal` 

* install eigen library: `brew install eigen`

* make sure it installed the correct links in `/usr/local/lib` and `/usr/local/include`

* make sure to build the project in 64-bit mode.

  ​

## Setting up IDE

In case you want to setup your own project instead of copying this example.

* create Project within <openframeworks_folder>/apps/myApps

* go to the projects build settings
  * add `/usr/local/lib` to Library Search Paths 

  * add `/usr/local/include` and `/usr/local/include/eigen3` to Header Search Paths

  * within *User Definded* Tab change where openframeworks looks for boost headers:
    ```
    HEADER_BOOST = "/usr/local/include"
    LIB_BOOST_SYSTEM = "/usr/local/lib/libboost_system.a"
    LIB_BOOST_FS = "/usr/local/lib/libboost_filesystem.a"
    ```

  * go to *Build Phases* Tab and add the following libraries from the folder */usr/local/lib*
      ```
      - bboost_system-mt.dylib
      - libboost_thread-mt.dylib
      - libCGAL_Core.dylib
      - libCGAL.dylib
      - libgmp.dylib
      - libmpfr.dylib
      ```

  * make sure language dialect is set to c++11 (should be the default setting)
