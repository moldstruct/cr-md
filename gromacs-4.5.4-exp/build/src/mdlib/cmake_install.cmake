# Install script for directory: /home/ibrahim/projects/iron-sulfur-project/hybrid-plasma-md/cretin_simulations/Method_paper/molecular_dynamics/GROMACS_hybrid_screening_potential/gromacs-4.5.4-exp/src/mdlib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules")
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

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libraries" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so.6" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so.6")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so.6"
         RPATH "/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so.6")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib" TYPE SHARED_LIBRARY FILES "/home/ibrahim/projects/iron-sulfur-project/hybrid-plasma-md/cretin_simulations/Method_paper/molecular_dynamics/GROMACS_hybrid_screening_potential/gromacs-4.5.4-exp/build/src/mdlib/libmd.so.6")
  if(EXISTS "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so.6" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so.6")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so.6"
         OLD_RPATH "/home/ibrahim/projects/iron-sulfur-project/hybrid-plasma-md/cretin_simulations/Method_paper/molecular_dynamics/GROMACS_hybrid_screening_potential/gromacs-4.5.4-exp/build/src/gmxlib:"
         NEW_RPATH "/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so.6")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libraries" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so"
         RPATH "/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib" TYPE SHARED_LIBRARY FILES "/home/ibrahim/projects/iron-sulfur-project/hybrid-plasma-md/cretin_simulations/Method_paper/molecular_dynamics/GROMACS_hybrid_screening_potential/gromacs-4.5.4-exp/build/src/mdlib/libmd.so")
  if(EXISTS "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so"
         OLD_RPATH "/home/ibrahim/projects/iron-sulfur-project/hybrid-plasma-md/cretin_simulations/Method_paper/molecular_dynamics/GROMACS_hybrid_screening_potential/gromacs-4.5.4-exp/build/src/gmxlib:"
         NEW_RPATH "/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/libmd.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "development" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/pkgconfig/libmd.pc")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/ibrahim/programs/gromacs-4.5.4-hybrid-screening-macromolecules/lib/pkgconfig" TYPE FILE RENAME "libmd.pc" FILES "/home/ibrahim/projects/iron-sulfur-project/hybrid-plasma-md/cretin_simulations/Method_paper/molecular_dynamics/GROMACS_hybrid_screening_potential/gromacs-4.5.4-exp/build/src/mdlib/libmd.pc")
endif()

