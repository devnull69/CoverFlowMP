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
