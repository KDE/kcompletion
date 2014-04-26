cmake_minimum_required(VERSION 2.8.12)

project(KCompletion)

# ECM setup
find_package(ECM 0.0.12 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMPackageConfigHelpers)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

include(ECMCreateQmFromPoFiles)

set(KF5_VERSION "4.98.0")

ecm_setup_version(
  ${KF5_VERSION}
  VARIABLE_PREFIX KCOMPLETION
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kcompletion_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5CompletionConfigVersion.cmake"
  SOVERSION 5)

# Dependencies
set(REQUIRED_QT_VERSION "5.2.0")

find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Widgets)

find_package(KF5Config ${KF5_VERSION} REQUIRED)
find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

file(GLOB PO_FILES "po/*.po")
if (PO_FILES)
  ecm_create_qm_from_po_files(PO_FILES ${PO_FILES} CATALOG_NAME kcompletion5_qt)
endif()

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Completion")

include(ECMPackageConfigHelpers)

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5CompletionConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5CompletionConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5CompletionConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5CompletionConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5CompletionTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5CompletionTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kcompletion_version.h
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR} COMPONENT Devel
)

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
