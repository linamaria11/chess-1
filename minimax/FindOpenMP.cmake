# - Finds OpenMP support
# This module can be used to detect OpenMP support in a compiler.
# If the compiler supports OpenMP, the flags required to compile with
# openmp support are set.  
#
# The following variables are set:
#   OpenMP_CXX_FLAGS - flags to add to the CXX compiler for OpenMP support
#   OPENMP_FOUND - true if openmp is detected
#
# Supported compilers can be found at http://openmp.org/wp/openmp-compilers/


# Copyright 2008, 2009 <André Rigland Brodtkorb> Andre.Brodtkorb@ifi.uio.no
#
# Redistribution AND use is allowed according to the terms of the New 
# BSD license. 
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


# Modified by Marc-Andre Gardner
# 31/07/2009
# Fix unset uses

include(CheckCXXSourceCompiles)
include(FindPackageHandleStandardArgs)

set(OpenMP_CXX_FLAG_CANDIDATES
  #Gnu
  "-fopenmp"
  #Intel
  "-openmp" 
  #Empty, if compiler automatically accepts openmp
  " "
)

set(OpenMP_LINK_FLAG_CANDIDATES
  #Gnu
  "-lgomp"
  #Intel
  "-liomp5 -lpthread"
  #Empty, if linker is really smart
  " "
)

# sample openmp source code to test
set(OpenMP_CXX_TEST_SOURCE 
"
#include <omp.h>
int main() { 
omp_set_num_threads(1);
return omp_get_num_threads();
}
")
# if these are set then do not try to find them again,
# by avoiding any try_compiles for the flags
if(DEFINED OpenMP_CXX_FLAGS)
  set(OpenMP_CXX_FLAG_CANDIDATES)
endif(DEFINED OpenMP_CXX_FLAGS)

if (DEFINED OpenMP_LINK_FLAGS)
  set(OpenMP_LINK_FLAG_CANDIDATES)
endif(DEFINED OpenMP_LINK_FLAGS)

# check cxx compiler
foreach(FLAG ${OpenMP_CXX_FLAG_CANDIDATES})
  set(SAFE_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
  set(CMAKE_REQUIRED_FLAGS "${FLAG}")
  set(OpenMP_FLAG_DETECTED)
  message(STATUS "Try OpenMP CXX flag = [${FLAG}]")

  foreach(LINK ${OpenMP_LINK_FLAG_CANDIDATES})
    set(SAFE_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
    set(CMAKE_REQUIRED_LIBRARIES "${LINK}")
    check_cxx_source_compiles("${OpenMP_CXX_TEST_SOURCE}" OpenMP_FLAG_DETECTED)
    set(CMAKE_REQUIRED_FLAGS "${SAFE_CMAKE_REQUIRED_FLAGS}")
    set(CMAKE_REQUIRED_LIBRARIES "${SAFE_CMAKE_REQUIRED_LIBRARIES}")
    if(OpenMP_FLAG_DETECTED)
      set(OpenMP_CXX_FLAGS_INTERNAL "${FLAG}")
      set(OpenMP_LINK_FLAGS_INTERNAL "${LINK}")
      message(STATUS "Found a working combination")
      set(BREAK "please")
      break()
    endif(OpenMP_FLAG_DETECTED)
  endforeach(LINK ${OpenMP_LINK_FLAG_CANDIDATES})
  if(BREAK)
    break()
  endif(BREAK)
endforeach(FLAG ${OpenMP_CXX_FLAG_CANDIDATES})

set(OpenMP_CXX_FLAGS "${OpenMP_CXX_FLAGS_INTERNAL}"
  CACHE STRING "C++ compiler flags for OpenMP parallization")
set(OpenMP_LINK_FLAGS "${OpenMP_LINK_FLAGS_INTERNAL}"
  CACHE STRING "C++ linker flags for OpenMP parallization")
# handle the standard arguments for find_package
find_package_handle_standard_args(OpenMP DEFAULT_MSG 
  OpenMP_CXX_FLAGS )

mark_as_advanced(
  OpenMP_CXX_FLAGS
  OpenMP_LINK_FLAGS
)
