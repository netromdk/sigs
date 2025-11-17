cmake_minimum_required(VERSION 3.20)
project(sigs)

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(policies NO_POLICY_SCOPE)

option(CODE_COVERAGE "Instrument for code coverage (clang only!)" OFF)
option(ADDRESS_SANITIZER "Use memory error detector  (clang only and implies debug mode!)" OFF)
option(THREAD_SANITIZER "Use thread data race detector  (clang only and implies debug mode!)" OFF)
option(UNDEFINED_SANITIZER "Use undefined behavior detector  (clang only and implies debug mode!)" OFF)
option(EVERY_WARNING "Use -Weverything (clang only)" OFF)

option(STATIC_ANALYZER "Do static analysis (gcc/clang only and implies debug mode!)" OFF)

option(USE_CCACHE "Use ccache (if found) to speed up compile and link" OFF)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Output compile_commands.json (Ninja generator only!)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

include(platform)
include(target)

include(ProcessorCount)
ProcessorCount(CPU_COUNT)
if (${CPU_COUNT} EQUAL 0)
  message("Could not determine processor count! Defaulting to 1.")
  set(CPU_COUNT 1)
endif()

if (CODE_COVERAGE)
  find_program(found_llvm_profdata llvm-profdata)
  find_program(found_llvm_cov llvm-cov)

  if (NOT found_llvm_profdata)
    message(WARNING "llvm-profdata not found! Is required for code coverage, disabling.")
    set(CODE_COVERAGE OFF)
  endif()

  if (NOT found_llvm_cov)
    message(WARNING "llvm-cov not found! Is required for code coverage, disabling.")
    set(CODE_COVERAGE OFF)
  endif()

  if (CODE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
  endif()
endif()

if (ADDRESS_SANITIZER)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
  set(CMAKE_BUILD_TYPE Debug)
endif()
if (THREAD_SANITIZER)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
  set(CMAKE_BUILD_TYPE Debug)
endif()
if (UNDEFINED_SANITIZER)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
  set(CMAKE_BUILD_TYPE Debug)
endif()

if (STATIC_ANALYZER)
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    find_program(found_scan_build scan-build)
    if (NOT found_scan_build)
      message(WARNING "scan-build not found! Is required for static analysis, disabling.")
      set(STATIC_ANALYZER OFF)
    else()
      set(CMAKE_BUILD_TYPE Debug)
    endif()
  elseif ( "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fanalyzer")
    set(CMAKE_BUILD_TYPE Debug)
  endif()
endif()

# Names
set(testName tests)

include(CTest)
enable_testing()
add_subdirectory(tests)

add_subdirectory(examples)

# Requires llvm/clang v4+!
# Setup: cmake -G <GENERATOR> -DCODE_COVERAGE=ON ../../
if (CODE_COVERAGE)
  add_custom_target(
    codecov

    rm -fr report.dir \; LLVM_PROFILE_FILE=tests.profraw $<TARGET_FILE:${testName}> >/dev/null && llvm-profdata merge -sparse tests.profraw -o tests.profdata && llvm-cov show $<TARGET_FILE:${testName}> -instr-profile=tests.profdata -format html -o report.dir -Xdemangler c++filt -Xdemangler -n -show-line-counts-or-regions && llvm-cov report $<TARGET_FILE:${testName}> -instr-profile=tests.profdata -Xdemangler c++filt -Xdemangler -n -use-color && echo '============================\\nReport at: report.dir/index.html\\n============================'

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating test code coverage report"
    DEPENDS ${testName}
    USES_TERMINAL
    )
endif()

# Setup: scan-build cmake -G <GENERATOR> -DSTATIC_ANALYZER=ON ../../
if (STATIC_ANALYZER AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  add_custom_target(
    sa

    scan-build -v -V -k cmake --build .

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Doing static analysis"
    USES_TERMINAL
    )
endif()

# cppcheck can be cloned and compiled: git clone git@github.com:danmar/cppcheck.git
# Remember to compile using CFGDIR=absolute-path-to-cfg-folder and HAVE_RULES=yes!
find_program(found_cppcheck cppcheck)
if (found_cppcheck)
  add_custom_target(
    cppcheck

    # unusedFunction can't be used with -j for some reason.
    cppcheck --enable=all -j ${CPU_COUNT} --std=c++20 --language=c++ --inconclusive
             --project=${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json --xml 2> cppcheck.xml &&
      python3 ${CMAKE_SOURCE_DIR}/scripts/cppcheck-htmlreport.py --file=cppcheck.xml --report-dir=cppcheck

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running cppcheck"
    USES_TERMINAL
    )
endif()

find_program(found_clang_tidy clang-tidy)
if (found_clang_tidy)
  add_custom_target(
    clang-tidy

    clang-tidy -p ${CMAKE_CURRENT_BINARY_DIR} sigs.h

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Checking with clang-tidy"
    USES_TERMINAL
    )

  add_custom_target(
    clang-tidy-fix

    clang-tidy -p ${CMAKE_CURRENT_BINARY_DIR} --fix sigs.h

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Fixing with clang-tidy"
    USES_TERMINAL
    )
endif()

message(STATUS "--------------------------------------------------")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if (APPLE)
  message(STATUS "macOS deployent SDK: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})")

if (USE_CCACHE)
  message(STATUS "CCache: ${CCACHE} (using to speed up compile + link)")
else()
  message(STATUS "CCache: OFF")
endif()

message(STATUS "Detected processor count: ${CPU_COUNT}")

if (found_cppcheck)
  message(STATUS "cppcheck target: enabled")
else()
  message(STATUS "cppcheck target: disabled (cppcheck not found)")
endif()

if (found_clang_tidy)
  message(STATUS "clang-tidy targets: clang-tidy, clang-tidy-fix")
else()
  message(STATUS "clang-tidy targets: disabled (clang-tidy not found)")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if (STATIC_ANALYZER)
    message(STATUS "Static analyzer: ${STATIC_ANALYZER} (-fanalyzer)")
  else()
    message(STATUS "Static analyzer: ${STATIC_ANALYZER}")
  endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if (CODE_COVERAGE)
    message(STATUS "Code coverage: ${CODE_COVERAGE} (target: codecov)")
  else()
    message(STATUS "Code coverage: ${CODE_COVERAGE}")
  endif()

  if (STATIC_ANALYZER)
    message(STATUS "Static analyzer: ${STATIC_ANALYZER} (target: sa)")
  else()
    message(STATUS "Static analyzer: ${STATIC_ANALYZER}")
  endif()

  message(STATUS "Sanitizers:")
  message(STATUS "  Address: ${ADDRESS_SANITIZER}")
  message(STATUS "  Thread: ${THREAD_SANITIZER}")
  message(STATUS "  Undefined: ${UNDEFINED_SANITIZER}")
  if ((ADDRESS_SANITIZER AND THREAD_SANITIZER) OR (ADDRESS_SANITIZER AND UNDEFINED_SANITIZER) OR
      (THREAD_SANITIZER AND UNDEFINED_SANITIZER))
    message(FATAL_ERROR "Sanitizers cannot be mixed! Only use one at a time.")
  endif()

  if (EVERY_WARNING)
    message(STATUS "-Weverything: ON")
  else()
    message(STATUS "-Weverything: OFF")
  endif()
endif()

message(STATUS "--------------------------------------------------")
