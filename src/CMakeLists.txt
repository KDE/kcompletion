ecm_create_qm_loader(kcompletion_QM_LOADER kcompletion5_qt)

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
  ${kcompletion_QM_LOADER}
)

add_library(KF5Completion ${kcompletion_SRCS})
add_library(KF5::Completion ALIAS KF5Completion)
ecm_generate_export_header(KF5Completion
    BASE_NAME KCompletion
    GROUP_BASE_NAME KF
    VERSION ${KF5_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 4.0 4.5 5.0 5.46
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF5Completion INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KCompletion>")

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

find_package(PythonModuleGeneration)

if (PythonModuleGeneration_FOUND)
  ecm_generate_python_binding(
    TARGET KF5::Completion
    PYTHONNAMESPACE PyKF5
    MODULENAME KCompletion
    RULES_FILE "${CMAKE_SOURCE_DIR}/cmake/rules_PyKF5.py"
    SIP_DEPENDS
      QtWidgets/QtWidgetsmod.sip
    HEADERS
      kcombobox.h
      kcompletion.h
      kcompletionbase.h
      kcompletionbox.h
      klineedit.h
      khistorycombobox.h
      kpixmapprovider.h
      ksortablelist.h
      kcompletionmatches.h
  )
endif()

install(TARGETS KF5Completion EXPORT KF5CompletionTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcompletion_export.h
  ${KCompletion_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KCompletion COMPONENT Devel
)

if(BUILD_DESIGNERPLUGIN)
    add_subdirectory(designer)
endif()

if(BUILD_QCH)
    ecm_add_qch(
        KF5Completion_QCH
        NAME KCompletion
        BASE_NAME KF5Completion
        VERSION ${KF5_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KCompletion_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt5Core_QCH
            Qt5Gui_QCH
            Qt5Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KCOMPLETION_EXPORT
            KCOMPLETION_DEPRECATED
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KCompletion LIB_NAME KF5Completion DEPS "widgets" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KCompletion)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
