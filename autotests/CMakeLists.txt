remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

find_package(Qt5Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

include(ECMAddTests)

ecm_add_tests(
   kcompletioncoretest.cpp
   klineedit_unittest.cpp
   kcombobox_unittest.cpp
   ksortablelisttest.cpp
   LINK_LIBRARIES Qt5::Test KF5::Completion
)
