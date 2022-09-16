add_library(KF5Completion)
add_library(KF5::Completion ALIAS KF5Completion)

ecm_create_qm_loader(KF5Completion kcompletion5_qt)

target_sources(KF5Completion PRIVATE
    kcombobox.cpp
    kcombobox.h
    kcombobox_p.h
    kcompletionbase.cpp
    kcompletionbase.h
    kcompletionbox.cpp
    kcompletionbox.h
    kcompletion.cpp
    kcompletion.h
    kcompletionmatches.cpp
    kcompletionmatches.h
    kcompletion_p.h
    khistorycombobox.cpp
    khistorycombobox.h
    klineedit.cpp
    klineedit.h
    klineedit_p.h
    kpixmapprovider.cpp
    kpixmapprovider.h
    ksortablelist.h
    kzoneallocator.cpp
    kzoneallocator_p.h
)

ecm_qt_declare_logging_category(KF5Completion
    HEADER kcompletion_debug.h
    IDENTIFIER KCOMPLETION_LOG
    CATEGORY_NAME kf.completion
    DESCRIPTION "KCompletion"
    EXPORT KCOMPLETION
)

ecm_generate_export_header(KF5Completion
    BASE_NAME KCompletion
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 4.0 4.5 5.0 5.46 5.66 5.81 5.83
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF5Completion INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCompletion>")

target_link_libraries(KF5Completion PUBLIC Qt${QT_MAJOR_VERSION}::Widgets
                                    PRIVATE KF5::ConfigCore      # KConfigGroup, used in many places
                                            KF5::ConfigGui       # KStandardShortcut
                                            KF5::WidgetsAddons   # KCursor
)

set_target_properties(KF5Completion PROPERTIES VERSION   ${KCOMPLETION_VERSION}
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

install(TARGETS KF5Completion EXPORT KF5CompletionTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcompletion_export.h
  ${KCompletion_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCompletion COMPONENT Devel
)

if(BUILD_DESIGNERPLUGIN)
    add_subdirectory(designer)
endif()

if(BUILD_QCH)
    ecm_add_qch(
        KF5Completion_QCH
        NAME KCompletion
        BASE_NAME KF5Completion
        VERSION ${KF_VERSION}
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
            "KCOMPLETION_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KCompletion LIB_NAME KF5Completion DEPS "widgets" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KCompletion)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

ecm_qt_install_logging_categories(
    EXPORT KCOMPLETION
    FILE kcompletion.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
