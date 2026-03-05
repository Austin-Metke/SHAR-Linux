# Install script for directory: /home/austinm/development/shar-linux/libs

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/austinm/development/shar-linux/build-linux/libs/choreo/cmake_install.cmake")
  include("/home/austinm/development/shar-linux/build-linux/libs/poser/cmake_install.cmake")
  include("/home/austinm/development/shar-linux/build-linux/libs/pure3d/cmake_install.cmake")
  include("/home/austinm/development/shar-linux/build-linux/libs/radcontent/cmake_install.cmake")
  include("/home/austinm/development/shar-linux/build-linux/libs/radcore/cmake_install.cmake")
  include("/home/austinm/development/shar-linux/build-linux/libs/radmath/cmake_install.cmake")
  include("/home/austinm/development/shar-linux/build-linux/libs/radmovie/cmake_install.cmake")
  include("/home/austinm/development/shar-linux/build-linux/libs/radmusic/cmake_install.cmake")
  include("/home/austinm/development/shar-linux/build-linux/libs/radscript/cmake_install.cmake")
  include("/home/austinm/development/shar-linux/build-linux/libs/radsound/cmake_install.cmake")
  include("/home/austinm/development/shar-linux/build-linux/libs/scrooby/cmake_install.cmake")
  include("/home/austinm/development/shar-linux/build-linux/libs/sim/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/austinm/development/shar-linux/build-linux/libs/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
