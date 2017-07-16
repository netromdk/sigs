# Creates "failtest_NAME" test from SOURCE file which will yield test success if cmake fails to
# compile the source code.
function(add_failtest name source)
  add_executable(
    _failtest_${name}
    ${source}
    )

  set_target_properties(
    _failtest_${name}
    PROPERTIES EXCLUDE_FROM_ALL TRUE
    )

  add_test(
    NAME failtest_${name}
    COMMAND
      ${CMAKE_COMMAND} --build .
      --target _failtest_${name}
      --config $<CONFIGURATION>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

  # Set that the test failing means success!
  set_tests_properties(
    failtest_${name}
    PROPERTIES WILL_FAIL TRUE
    )
endfunction()
