cmake_minimum_required(VERSION 3.21.1)
project(ImagoRef VERSION 1.0 LANGUAGES CXX)

set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum OS X deployment version" FORCE)
add_compile_options(-mmacosx-version-min=11.0)
add_link_options(-mmacosx-version-min=11.0)

set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for macOS")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Svg DBus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

qt_add_executable(ImagoRef WIN32 MACOSX_BUNDLE)

if(APPLE)
    set_target_properties(ImagoRef PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )
endif()

if(WIN32)
    target_sources(ImagoRef PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc
    )
endif()

target_sources (ImagoRef PRIVATE
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/app/mainwindow.cpp
    ${SRC_DIR}/app/mainwindow.h
    ${SRC_DIR}/app/undocommands.cpp
    ${SRC_DIR}/app/undocommands.h
    ${SRC_DIR}/widgets/floatingtoolbar.cpp
    ${SRC_DIR}/widgets/floatingtoolbar.h
    ${SRC_DIR}/widgets/settingsdialog.cpp
    ${SRC_DIR}/widgets/settingsdialog.h
    ${SRC_DIR}/graphics/canvasview.cpp
    ${SRC_DIR}/graphics/canvasview.h
    ${SRC_DIR}/graphics/imageitem.cpp
    ${SRC_DIR}/graphics/imageitem.h
    ${SRC_DIR}/../res/resources.qrc
)

target_include_directories(ImagoRef PRIVATE
    ${SRC_DIR}/app
    ${SRC_DIR}/widgets
    ${SRC_DIR}/graphics
)

target_link_libraries(ImagoRef PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Gui
    Qt::Svg
    Qt::DBus
)

if(APPLE)
    add_custom_command(TARGET ImagoRef POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/res/app_icon/icon.icns"
            "$<TARGET_BUNDLE_DIR:ImagoRef>/Contents/Resources/icon.icns"

        COMMAND Qt6::macdeployqt
            "$<TARGET_BUNDLE_DIR:ImagoRef>"
            -verbose=1

        COMMAND codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:ImagoRef>"
    )
endif()

include(GNUInstallDirs)

install(TARGETS ImagoRef
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
