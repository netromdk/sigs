include(failtest)

add_failtest(
  VoidSignalNonVoidRetFunc
  VoidSignalNonVoidRetFunc.cc
  )

add_failtest(
  SignalBlockerNonSignal
  SignalBlockerNonSignal.cc
  )

add_failtest(
  SignalBlockerFakeSignal
  SignalBlockerFakeSignal.cc
  )

add_failtest(
  SignalMapperAddUnknownSignalType
  SignalMapperAddUnknownSignalType.cc
  )
