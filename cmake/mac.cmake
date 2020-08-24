set(SDK_MIN 10.8)

# Try to pick the newest!
set(SDKS 10.15 10.14 10.13 10.12 10.11 10.10 10.9 10.8)

set(SDK_PATHS "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/"
              "/Library/Developer/CommandLineTools/SDKs/"
              "/Developer/SDKs/")

# Force a specific version if defined.
if (FORCE_MAC_SDK)
  message("Trying to force Mac SDK: ${FORCE_MAC_SDK}")
  set(SDKS ${FORCE_MAC_SDK})
endif()

foreach (sdk_path ${SDK_PATHS})
  foreach(sdk ${SDKS})
    set(DEV_SDK "${sdk_path}MacOSX${sdk}.sdk")
    if (EXISTS "${DEV_SDK}" AND IS_DIRECTORY "${DEV_SDK}")
      set(found_sdk YES)
      break()
    endif()
  endforeach()

  if (found_sdk)
    break()
  endif()
endforeach()

if (NOT found_sdk)
  message(FATAL_ERROR "Could not find Mac OS X SDK version 10.8 - 10.15!")
endif()

add_definitions(
  -DMAC
  )

set(CMAKE_OSX_SYSROOT ${DEV_SDK})
set(CMAKE_OSX_DEPLOYMENT_TARGET ${SDK_MIN})

# Silence annoying "ranlib: file: ...o has no symbols" warnings! On Mac the ar utility will run
# ranlib, which causes the warning. Using the "Src" argument stops ar from executing ranlib, and
# when ranlib is executed normally we give it "-no_warning_for_no_symbols" to suppress the warning.
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
