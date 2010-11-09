# This File is part of NetNewsTicker
# (c)2009 Christian Taubenheim
# www.netnewsticker.org
 
# This program is free software. you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.

project(netnewsticker)
 
# Find the required Libaries
find_package(KDE4 REQUIRED)
find_package(KdepimLibs REQUIRED)

include(KDE4Defaults)
ADD_DEFINITIONS(-DCOMPILE_USE_QT4)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

add_subdirectory(po)
add_subdirectory(icons)

# We add our source code here
set(NetNewsTicker_SRCS src/netnewsticker.cpp
	src/item.cpp
	src/newsfeedmanager.cpp
	src/feedsettingswidget.cpp
	src/filtersettingswidget.cpp
	src/settingsdialog.cpp
	)
 
# Now make sure all files get to the right place
kde4_add_kcfg_files(NetNewsTicker_SRCS src/settings.kcfgc)
kde4_add_ui_files(NetNewsTicker_SRCS src/visualsettings.ui src/feedsettings.ui src/feeddirectory.ui src/filtersettings.ui)
kde4_add_plugin(plasma_applet_netnewsticker ${NetNewsTicker_SRCS})
target_link_libraries(plasma_applet_netnewsticker 
                      ${KDE4_PLASMA_LIBS} 
                      ${KDE4_KDEUI_LIBS} 
                      ${QT_QTXML_LIBRARY} 
                      ${KDE4_SYNDICATION_LIBS}
                      ${QT_QTNETWORK_LIBRARY}
                      ${KDE4_KIO_LIBRARY})
 
install(TARGETS plasma_applet_netnewsticker
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-netnewsticker.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

############################################################
##
##    Package Generator
##
IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  MESSAGE(STATUS "CMakeRoot: ${CMAKE_ROOT}/Modules/CPack.cmake")
  INCLUDE(InstallRequiredSystemLibraries)

  SET(CPACK_SET_DESTDIR "on")
  SET(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
  SET(CPACK_GENERATOR "DEB;RPM;STGZ;TBZ2;TGZ;TZ;ZIP;SOURCE_TGZ;SOURCE_TZ;SOURCE_ZIP;")
  SET(CPACK_SOURCE_IGNORE_FILES 
      "/build/"
      "/.svn/"
      ".kdev4$"
      "~$"
      )

  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY	"NetNewsTicker")

  SET(CPACK_PACKAGE_VENDOR		"")
  SET(CPACK_PACKAGE_CONTACT		"")
#	SET(CPACK_RESOURCE_FILE_LICENSE		${CMAKE_CURRENT_SOURCE_DIR}//COPYING)
  SET(CPACK_PACKAGE_VERSION_MAJOR	${MAJOR_VERSION})
  SET(CPACK_PACKAGE_VERSION_MINOR	${MINOR_VERSION})	# to create a debian confirm file-version-string
  STRING(REGEX REPLACE "_" "-" CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
  SET(CPACK_PACKAGE_FILE_NAME		${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}_${CMAKE_SYSTEM_PROCESSOR})
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME	${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}_source)

  ##	LIST EXECUTABLES
  SET(CPACK_STRIP_FILES "lib//plasma_plasma_applet_netnewsticker.so")
  SET(CPACK_COMPONENTS_ALL Libraries ApplicationData)

  ##	DEBIAN SPECIFIC SETTINGS
  ##
  SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION	"A scrolling RSS ticker")
  SET(CPACK_DEBIAN_PACKAGE_PRIORITY	"optional")
  SET(CPACK_DEBIAN_PACKAGE_SECTION	"kde")
  SET(CPACK_DEBIAN_PACKAGE_DEPENDS	"kdebase-runtime (>= 4:4.2.1), kdelibs5 (>= 4:4.2.1), libc6 (>= 2.1.3), libgcc1 (>= 1:4.1.1), libplasma3, libqt4-dbus (>= 4.5.0~+rc1), libqt4-network (>= 4.5.0~+rc1), libqt4-svg (>= 4.5.0~+rc1), libqt4-xml (>= 4.5.0~+rc1), libqtcore4 (>= 4.5.0~+rc1), libqtgui4 (>= 4.5.0~+rc1), libstdc++6 (>= 4.2.1)")

  FIND_PROGRAM(DPKG_CMD dpkg)
  IF(DPKG_CMD)
    EXECUTE_PROCESS(COMMAND "${DPKG_CMD}" --print-architecture
      OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    SET(CPACK_DEBIAN_PACKAGE_NAME	${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
  ENDIF(DPKG_CMD)

  ##	RPM SPECIFIC SETTINGS
  ##

  INCLUDE(CPack)
ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")

