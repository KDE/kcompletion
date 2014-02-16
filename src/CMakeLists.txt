set(kcompletion_SRCS
  kcombobox.cpp
  kcompletion.cpp
  kcompletionbase.cpp
  kcompletionbox.cpp
  klineedit.cpp
  khistorycombobox.cpp
  kpixmapprovider.cpp
  kzoneallocator.cpp
  kcompletionbase.cpp
  kcompletionmatches.cpp
)

add_library(KF5Completion ${kcompletion_SRCS})
generate_export_header(KF5Completion BASE_NAME KCompletion)
add_library(KF5::Completion ALIAS KF5Completion)

target_include_directories(KF5Completion INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/KCompletion>")

target_link_libraries(KF5Completion PUBLIC Qt5::Widgets
                                    PRIVATE KF5::ConfigCore      # KConfigGroup, used in many places
                                            KF5::ConfigGui       # KStandardShortcut
                                            KF5::WidgetsAddons   # KCursor
)

set_target_properties(KF5Completion PROPERTIES VERSION   ${KCOMPLETION_VERSION_STRING}
                                               SOVERSION ${KCOMPLETION_SOVERSION}
                                               EXPORT_NAME Completion
)

ecm_generate_headers(KCompletion_HEADERS
  HEADER_NAMES
  KComboBox
  KCompletion
  KCompletionBase
  KCompletionBox
  KLineEdit
  KHistoryComboBox
  KPixmapProvider
  KSortableList
  KCompletionMatches

  REQUIRED_HEADERS KCompletion_HEADERS
)

install(TARGETS KF5Completion EXPORT KF5CompletionTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcompletion_export.h
  ${KCompletion_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/KCompletion COMPONENT Devel
)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KCompletion LIB_NAME KF5Completion DEPS "widgets" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
