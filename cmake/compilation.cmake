set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_COMPILER_WARNINGS "-Wno-unused-parameter -Wempty-body -Woverloaded-virtual -Wtautological-compare -Wshadow -Wmissing-noreturn -Wdouble-promotion")
set(CLANG_WARNINGS "-Wnull-arithmetic -Woverriding-method-mismatch -Wcovered-switch-default -Wzero-as-null-pointer-constant -Wweak-vtables -Wunused-private-field -Wno-covered-switch-default -Wmismatched-tags")
set(GCC_WARNINGS "-Wuseless-cast")
set(GCC10_WARNINGS "-Wmismatched-tags -Wredundant-tags")

# Warnings turned into errors.
set(COMMON_COMPILER_ERRORS "-Werror=return-type -Werror=delete-incomplete -Werror=delete-non-virtual-dtor -Werror=float-equal -Werror=reorder -Werror=uninitialized -Werror=unreachable-code -Werror=switch")
set(CLANG_ERRORS "-Werror=inconsistent-missing-override -Werror=inconsistent-missing-destructor-override -Werror=division-by-zero -Werror=return-stack-address -Werror=pessimizing-move -Werror=infinite-recursion -Werror=nonportable-include-path")
set(GCC_ERRORS "")
set(GCC9_ERRORS "-Werror=pessimizing-move")

# Some release optimization flags for GCC/Clang/MSVC.
if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -pedantic -pedantic-errors ${COMMON_COMPILER_WARNINGS} ${COMMON_COMPILER_ERRORS}")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
else()
  # /EHsc = enables unwind semantics when exception handler is used.
  # Ignore certain warnings:
  #   C4514: 'function' : unreferenced inline function has been removed
  #   C4820: bytes' bytes padding added after construct 'member_name'
  #   C4710: 'function' : function not inlined
  #   C4711: function 'function' selected for inline expansion
  #   C4668: 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
  set(CMAKE_CXX_FLAGS "/Wall /EHsc /wd4514 /wd4820 /wd4710 /wd4711 /wd4668")
endif()

# Show color in diagnostics messages from Clang.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
endif()

if (NOT MSVC)
  # Clang/GCC
  set(REL_OPTS "-O3 -pipe -fno-exceptions -fvisibility=hidden -fvisibility-inlines-hidden -ffast-math -funroll-loops")

  # GCC only
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(REL_OPTS "${REL_OPTS} -fno-implement-inlines")

  # Clang only
  elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(REL_OPTS "${REL_OPTS}")

    if (EVERY_WARNING)
      # But ignore some warnings.
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-padded -Wno-shadow-field-in-constructor -Wno-c++98-compat-pedantic")
      # These happen in gtest which we don't care about:
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-global-constructors -Wno-used-but-marked-unused")
    endif()
  endif()

  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${REL_OPTS}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${REL_OPTS}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${REL_OPTS}")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  set(target_version "12.0")
  if (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER ${target_version} OR
           CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL ${target_version}))
    message(FATAL_ERROR "Requires GCC >= ${target_version}.")
  endif()
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0 OR
      CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 9.0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC9_ERRORS}")
  endif()
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 10.0 OR
      CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 10.0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC10_WARNINGS}")
  endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  # Xcode 15 was based on Clang 15 which has full C++20 support (previously the Xcode version was
  # ahead of the Clang version).
  if (APPLE AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang")
    set(target_version "15.0")
  else()
    set(target_version "15")
  endif()

  if (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL ${target_version} OR
           CMAKE_CXX_COMPILER_VERSION VERSION_GREATER ${target_version}))
    message(FATAL_ERROR "Requires ${CMAKE_CXX_COMPILER_ID} >= ${target_version}")
  elseif (APPLE)
    # Use libstdc++ on Linux but libc++ on macOS.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_WARNINGS} ${CLANG_ERRORS}")
elseif (MSVC AND (${MSVC_VERSION} GREATER_EQUAL 1929))
  # Requires at least VS2019 (v1929).
  # C++20 support is implicitly enabled.
else()
  message(FATAL_ERROR "Your compiler does not support C++20 - aborting!")
endif()

# Detect if ccache is installed and use if it is the case.
if (USE_CCACHE)
  find_program(CCACHE ccache)
  if (CCACHE)
    # Specify to launch ccache for compilation and linking globally.
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
  endif()
endif()
