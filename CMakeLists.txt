cmake_minimum_required(VERSION 3.16)

project(CoverFlowMP VERSION 0.1 LANGUAGES CXX)

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
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MPV REQUIRED mpv)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(CoverFlowMPApp
    main.cpp

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

    src/persistence/ResumeRepository.h
    src/persistence/ResumeRepository.cpp

    resources.qrc
)

target_include_directories(CoverFlowMPApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${MPV_INCLUDE_DIRS}
)

target_compile_options(CoverFlowMPApp PRIVATE
    ${MPV_CFLAGS_OTHER}
)

target_link_directories(CoverFlowMPApp PRIVATE
    ${MPV_LIBRARY_DIRS}
)

target_link_libraries(CoverFlowMPApp PRIVATE
    Qt6::Core
    Qt6::Concurrent
    Qt6::DBus
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::Sql
    ${MPV_LIBRARIES}
)

install(TARGETS CoverFlowMPApp
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/packaging/coverflowmp.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/appicon.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
    RENAME coverflowmp.png
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
        libqt6sql6-sqlite
        qml6-module-qt5compat-graphicaleffects
        qml6-module-qtqml
        qml6-module-qtqml-workerscript
        qml6-module-qtquick
        qml6-module-qtquick-controls
        qml6-module-qtquick-layouts
        qml6-module-qtquick-templates
        qml6-module-qtquick-window
    )
    list(JOIN COVERFLOWMP_DEBIAN_RUNTIME_DEPENDS ", " CPACK_DEBIAN_PACKAGE_DEPENDS)
endif()

include(CPack)
