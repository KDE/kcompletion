remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

find_package(Qt${QT_MAJOR_VERSION}Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT Qt${QT_MAJOR_VERSION}Test_FOUND)
    message(STATUS "Qt${QT_MAJOR_VERSION}Test not found, autotests will not be built.")
    return()
endif()

include(ECMAddTests)

ecm_add_tests(
   kcompletioncoretest.cpp
   klineedit_unittest.cpp
   kcombobox_unittest.cpp
   ksortablelisttest.cpp
   LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF5::Completion
)
