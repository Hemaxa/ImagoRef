cmake_minimum_required(VERSION 3.19)
project(ImagoRef VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

qt_standard_project_setup()

qt_add_executable(ImagoRef
    WIN32 MACOSX_BUNDLE
    ${SOURCE_DIR}/main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(ImagoRef
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Gui
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
