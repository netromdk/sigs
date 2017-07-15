include_directories(
  gtest/
  gtest/include
  ${CMAKE_SOURCE_DIR}
  )

add_library(
  gtest
  gtest/src/gtest-all.cc
  )

add_executable(
  ${testName}

  main.cc
  )

add_test(
  NAME ${testName}
  COMMAND ${testName}
  )

target_link_libraries(
  ${testName}
  gtest
  )
