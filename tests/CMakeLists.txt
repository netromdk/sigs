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

add_executable(
  ${testName}

  main.cc

  General.cc
  Interface.cc
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
