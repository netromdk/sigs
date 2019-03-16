set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Some release optimization flags for GCC/Clang.
if (NOT WIN32)
  # Always show all warnings.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -pedantic")

  # Certain warnings are promoted to errors.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=unused-variable -Werror=unused-value -Werror=return-type")

  # Clang/GCC
  set(REL_OPTS "-pipe -fvisibility=hidden -fvisibility-inlines-hidden -ffast-math -funroll-loops")

  # Clang only
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion -Werror=conversion -Wno-sign-conversion -Werror=nonportable-include-path -Werror=pessimizing-move -Werror=infinite-recursion")
  endif()

  # GCC only
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(REL_OPTS "${REL_OPTS} -fno-implement-inlines")
  endif()

  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${REL_OPTS}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${REL_OPTS}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${REL_OPTS}")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  set(target_version "7.1")
  if (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER ${target_version} OR
           CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL ${target_version}))
    message(FATAL_ERROR "Requires GCC >= ${target_version}.")
  endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  # Xcode 10 was based on Clang 6 which has full C++17 support.
  if (APPLE AND "${CMAKE_CXX_COMPILER}" MATCHES "Xcode")
    set(target_version "10.0")
  else()
    set(target_version "6")
  endif()

  if (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL ${target_version} OR
           CMAKE_CXX_COMPILER_VERSION VERSION_GREATER ${target_version}))
    message(FATAL_ERROR "Requires ${CMAKE_CXX_COMPILER_ID} >= ${target_version}")
  elseif (APPLE)
    # Use libstdc++ on Linux but libc++ on macOS.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
elseif (MSVC AND (${MSVC_VERSION} GREATER_EQUAL 1910))
  # Requires at least VS2017 (v1910).
  # C++17 support is implicitly enabled.
else()
  message(FATAL_ERROR "Your compiler does not support C++17 - aborting!")
endif()
