# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.
add_subdirectory(inject)
add_subdirectory(otherlib)



set(distribution_DIR ${CMAKE_SOURCE_DIR}/distribution)



# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
#include_directories($JNI_H_INCLUDE)
find_library( # Sets the name of the path variable.
              log-lib
              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )
#find_library(test1-lib test1)
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

add_library(lib_test1 SHARED IMPORTED)
set_target_properties(lib_test1 PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/lib/${ANDROID_ABI}/libtest1.so)

add_library(lib_inject SHARED IMPORTED)
set_target_properties(lib_inject PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/lib/${ANDROID_ABI}/libinject.so)


add_library( # Sets the name of the library.
             Loadlib
             # Sets the library as a shared library.
             SHARED
             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             zybornit_loadlib_LoadLibrary.cpp
             linker.cpp
             dlfcn.cpp
             libc_logging.cpp
             demo.c
              linker_phdr.cpp)

target_include_directories(Loadlib PRIVATE
                           ${distribution_DIR}/include)

target_link_libraries( # Specifies the target library.
                     Loadlib
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       #${test1-lib}
                       #test1
                       lib_test1
                       lib_inject
                       )

set_target_properties(Loadlib
                      PROPERTIES
                      LIBRARY_OUTPUT_DIRECTORY
                      "${distribution_DIR}/lib/${ANDROID_ABI}")
