if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX 1)
endif()

include(compilation)

if (APPLE)
  include(mac)
endif()
