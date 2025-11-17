include_directories(
  ${CMAKE_SOURCE_DIR}
  )

# Include gtest as system headers so no warnings will be shown.
include_directories(
  SYSTEM
  gtest/
  gtest/include
  )

add_library(
  gtest
  gtest/src/gtest-all.cc
  )

# Ignore all warnings from gtest code.
target_compile_options(
  gtest
  PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-w>
  $<$<CXX_COMPILER_ID:GNU>:-w>
  $<$<CXX_COMPILER_ID:MSVC>:/W0>
  )

add_executable(
  ${testName}

  main.cc

  General.cc
  Interface.cc
  SignalBlocker.cc
  CustomTypes.cc
  )

add_test(
  NAME ${testName}
  COMMAND ${testName}
  )

target_link_libraries(
  ${testName}
  gtest
  )

if (LINUX)
  target_link_libraries(
    ${testName}
    -pthread
    )
endif()

add_subdirectory(failtests)
