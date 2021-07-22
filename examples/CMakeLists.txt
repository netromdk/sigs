include_directories(
  ${CMAKE_SOURCE_DIR}
  )

add_executable(
  button
  Button.cc
  )

add_executable(
  interface
  Interface.cc
  )

add_executable(
  return_values
  ReturnValues.cc
  )

add_executable(
  signal_mapper
  SignalMapper.cc
  )

add_custom_target(
  run_examples
  COMMAND $<TARGET_FILE:button>
  COMMAND $<TARGET_FILE:interface>
  COMMAND $<TARGET_FILE:return_values>
  COMMAND $<TARGET_FILE:signal_mapper>
  USES_TERMINAL
  )

add_dependencies(
  run_examples
  button
  interface
  return_values
  signal_mapper
  )
