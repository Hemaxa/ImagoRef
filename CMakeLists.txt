cmake_minimum_required(VERSION 3.19)
project(ImagoRef VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Svg)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

qt_add_executable(ImagoRef WIN32 MACOSX_BUNDLE)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/res/resources.qrc
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
)

include(GNUInstallDirs)

install(TARGETS ImagoRef
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ImagoRef
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
