set(CMAKE_CXX_FLAGS "-Wall -std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# Some release optimization flags for GCC/Clang.
if (NOT WIN32)
  # Clang/GCC
  set(REL_OPTS "-pipe -fvisibility=hidden -fvisibility-inlines-hidden -ffast-math -funroll-loops")

  # GCC only
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(REL_OPTS "${REL_OPTS} -fno-implement-inlines")
  endif()

  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${REL_OPTS}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${REL_OPTS}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${REL_OPTS}")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION
    )
  if (NOT (GCC_VERSION VERSION_GREATER 5.0 OR GCC_VERSION VERSION_EQUAL 5.0))
    message(FATAL_ERROR "Requires GCC >= 5.0.")
  endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
    OUTPUT_VARIABLE CLANG_VERSION
    )
  if (NOT (CLANG_VERSION VERSION_GREATER 3.7 OR CLANG_VERSION VERSION_EQUAL 3.7))
    message(FATAL_ERROR "Requires Clang >= 3.7.")
  elseif (APPLE)
    # Use libstdc++ on Linux but libc++ on macOS.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
elseif (MSVC AND MSVC14)
  # C++14 support is implicitly enabled.
else()
  message(FATAL_ERROR "Your compiler does not support C++14 - aborting!")
endif()
