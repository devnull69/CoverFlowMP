cmake_minimum_required(VERSION 3.16)

set(COVERFLOWMP_PACKAGE_VERSION "2.0.0")
set(COVERFLOWMP_VERSION "2.0.0")
set(COVERFLOWMP_CHANGELOG "${CMAKE_CURRENT_SOURCE_DIR}/debian/changelog")

if(EXISTS "${COVERFLOWMP_CHANGELOG}")
    file(STRINGS "${COVERFLOWMP_CHANGELOG}" COVERFLOWMP_CHANGELOG_FIRST_LINE LIMIT_COUNT 1)
    string(REGEX REPLACE "^[^(]+\\(([^)]+)\\).*$" "\\1"
        COVERFLOWMP_PACKAGE_VERSION
        "${COVERFLOWMP_CHANGELOG_FIRST_LINE}"
    )
    string(REGEX REPLACE "^([0-9]+(\\.[0-9]+)*)[~+-]?.*$" "\\1"
        COVERFLOWMP_VERSION
        "${COVERFLOWMP_PACKAGE_VERSION}"
    )
endif()

project(CoverFlowMP VERSION ${COVERFLOWMP_VERSION} LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Concurrent
    Core
    DBus
    Gui
    Quick
    Qml
    Sql
    Widgets
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MPV REQUIRED mpv)

qt_standard_project_setup(REQUIRES 6.5)

add_library(CoverFlowShared STATIC
    src/shared/AppConfig.h
    src/shared/AppConfig.cpp
    src/shared/AppPaths.h
    src/shared/AppPaths.cpp
    src/shared/AppLaunchGuard.h
    src/shared/AppLaunchGuard.cpp
    src/shared/AppDatabase.h
    src/shared/AppDatabase.cpp
)

target_include_directories(CoverFlowShared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(CoverFlowShared PUBLIC
    Qt6::Core
    Qt6::Sql
)

qt_add_executable(CoverFlowMPApp
    src/app/main.cpp
    src/app/AppController.h
    src/app/AppController.cpp

    src/platform/SessionInhibitor.h
    src/platform/SessionInhibitor.cpp

    src/media/VideoItem.h
    src/media/VideoLibraryModel.h
    src/media/VideoLibraryModel.cpp
    src/media/LibraryScanner.h
    src/media/LibraryScanner.cpp
    src/media/ThumbnailService.h
    src/media/ThumbnailService.cpp

    src/player/MpvObject.h
    src/player/MpvObject.cpp
    src/player/PlayerController.h
    src/player/PlayerController.cpp
    src/player/SkipRange.h

    resources.qrc
)

qt_add_executable(CoverFlowMPConfigurator
    src/configurator/main.cpp
    src/configurator/ConfiguratorController.h
    src/configurator/ConfiguratorController.cpp
    resources.qrc
)

target_include_directories(CoverFlowMPApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${MPV_INCLUDE_DIRS}
)

target_include_directories(CoverFlowMPConfigurator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(CoverFlowMPApp PRIVATE
    ${MPV_CFLAGS_OTHER}
)

target_link_directories(CoverFlowMPApp PRIVATE
    ${MPV_LIBRARY_DIRS}
)

target_link_libraries(CoverFlowMPApp PRIVATE
    CoverFlowShared
    Qt6::Core
    Qt6::Concurrent
    Qt6::DBus
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::Widgets
    ${MPV_LIBRARIES}
)

target_link_libraries(CoverFlowMPConfigurator PRIVATE
    CoverFlowShared
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::Widgets
)

target_compile_definitions(CoverFlowMPApp PRIVATE
    COVERFLOWMP_APP_VERSION="${COVERFLOWMP_PACKAGE_VERSION}"
)

install(TARGETS CoverFlowMPApp
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS CoverFlowMPConfigurator
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/packaging/coverflowmp.desktop
    ${CMAKE_CURRENT_SOURCE_DIR}/packaging/coverflowmp-configurator.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/appicon.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
    RENAME coverflowmp.png
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/configurator-icon.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
    RENAME coverflowmp-configurator.svg
)

set(CPACK_PACKAGE_NAME "coverflowmp")
set(CPACK_PACKAGE_VENDOR "devnull69")
set(CPACK_PACKAGE_CONTACT "devnull69")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CoverFlow-based media center")
set(CPACK_PACKAGE_DESCRIPTION
    "CoverFlowMP is a keyboard-driven video media center for Linux."
)
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_SECTION "video")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

    # dpkg-shlibdeps sees ELF links like libmpv and the Qt base libraries, but it
    # cannot infer QML imports or SQL driver plugins loaded at runtime.
    set(COVERFLOWMP_DEBIAN_RUNTIME_DEPENDS
        ffmpeg
        ffmpegthumbnailer
        libqt6sql6-sqlite
        qml6-module-qt5compat-graphicaleffects
        qml6-module-qtqml-models
        qml6-module-qtqml
        qml6-module-qtqml-workerscript
        qml6-module-qtquick
        qml6-module-qtquick-controls
        qml6-module-qtquick-layouts
        qml6-module-qtquick-shapes
        qml6-module-qtquick-templates
        qml6-module-qtquick-window
    )
    list(JOIN COVERFLOWMP_DEBIAN_RUNTIME_DEPENDS ", " CPACK_DEBIAN_PACKAGE_DEPENDS)
endif()

include(CPack)
