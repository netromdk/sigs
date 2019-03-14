cmake_minimum_required(VERSION 3.0)
project(sigs)

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(CODE_COVERAGE "Instrument for code coverage (clang only!)" OFF)
option(ADDRESS_SANITIZER "Use memory error detector  (clang only and implies debug mode!)" OFF)
option(STATIC_ANALYZER "Do static analysis (clang only and implies debug mode!)" OFF)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Output compile_commands.json (Ninja generator only!)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

include(platform)
include(target)

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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -g -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
  set(CMAKE_BUILD_TYPE Debug)
endif()

if (STATIC_ANALYZER)
  find_program(found_scan_build scan-build)
  if (NOT found_scan_build)
    message(WARNING "scan-build not found! Is required for static analysis, disabling.")
    set(STATIC_ANALYZER OFF)
  else()
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
    )
endif()

# Setup: scan-build cmake -G <GENERATOR> -DSTATIC_ANALYZER=ON ../../
if (STATIC_ANALYZER)
  add_custom_target(
    sa

    scan-build -v -V -k cmake --build .

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Doing static analysis"
    )
endif()

# cppcheck can be cloned and compiled: git clone git@github.com:danmar/cppcheck.git
# Remember to compile using CFGDIR=absolute-path-to-cfg-folder and HAVE_RULES=yes!
find_program(found_cppcheck cppcheck)
if (found_cppcheck)
  add_custom_target(
    cppcheck

    cppcheck --enable=all --force -j 4 --std=c++14 --language=c++ --project=${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/sigs.h --xml 2> cppcheck.xml && python ${CMAKE_SOURCE_DIR}/scripts/cppcheck-htmlreport.py --file=cppcheck.xml --report-dir=cppcheck

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running cppcheck"
    )
endif()

message(STATUS "--------------------------------------------------")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if (CODE_COVERAGE)
  message(STATUS "Code coverage: ${CODE_COVERAGE} (target: codecov)")
else()
  message(STATUS "Code coverage: ${CODE_COVERAGE}")
endif()

message(STATUS "Address sanitizer: ${ADDRESS_SANITIZER}")

if (STATIC_ANALYZER)
  message(STATUS "Static analyzer: ${STATIC_ANALYZER} (target: sa)")
else()
  message(STATUS "Static analyzer: ${STATIC_ANALYZER}")
endif()

if (found_cppcheck)
  message(STATUS "cppcheck target: enabled")
else()
  message(STATUS "cppcheck target: disabled (cppcheck not found)")
endif()

message(STATUS "--------------------------------------------------")
