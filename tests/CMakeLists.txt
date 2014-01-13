remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include(ECMMarkAsTest)

find_package(Qt5Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, manual tests will not be built.")
    return()
endif()

macro(kcompletion_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test Qt5::Widgets KF5::Completion KF5::ConfigCore)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kcompletion_executable_tests(
   kcomboboxtest
   kcompletionuitest
   klineedittest
)
