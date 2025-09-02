#определение минимальной версии CMake
cmake_minimum_required(VERSION 3.21.1)

#определение версии проекта
project(ImagoRef VERSION 1.0 LANGUAGES CXX)

#определение cтандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#определение используемых пакетов
find_package(Qt5 COMPONENTS Core Gui Widgets Svg REQUIRED)

#включение Qt-автоматизаций
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#создание переменной для src директории
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

#создание исполняемого файла
add_executable(ImagoRef)

#обозначение исходных файлов
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

#директории с заголовочными файлами
target_include_directories(ImagoRef PRIVATE
    ${SRC_DIR}/app
    ${SRC_DIR}/widgets
    ${SRC_DIR}/graphics
)

# ====================================================================
# НАСТРОЙКИ ДЛЯ MACOS
# ====================================================================
if(APPLE)
    find_package(OpenGL REQUIRED)
    find_package(ZLIB REQUIRED)

    #полный список библиотек для macOS
    target_link_libraries(ImagoRef PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::Svg
        OpenGL::GL
        ZLIB::ZLIB
        "-framework AppKit"
        "-framework Foundation"
        "-framework IOKit"
        "-framework CoreGraphics"
    )

    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version" FORCE)

    set_target_properties(ImagoRef PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )

    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt)

    add_custom_command(TARGET ImagoRef POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/res/app_icon/icon.icns" "$<TARGET_BUNDLE_DIR:ImagoRef>/Contents/Resources/icon.icns"
    )

    find_program(PKGBUILD_EXECUTABLE pkgbuild)
    find_program(PRODUCTBUILD_EXECUTABLE productbuild)
    if(PKGBUILD_EXECUTABLE AND PRODUCTBUILD_EXECUTABLE)
        add_custom_target(package
            COMMAND ${MACDEPLOYQT_EXECUTABLE} "$<TARGET_BUNDLE_DIR:ImagoRef>" -verbose=1

            COMMAND codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:ImagoRef>"

            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/package/root"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_BUNDLE_DIR:ImagoRef>" "${CMAKE_BINARY_DIR}/package/root/ImagoRef.app"
            COMMAND ${PKGBUILD_EXECUTABLE} --root "${CMAKE_BINARY_DIR}/package/root" --identifier "com.imago.ref" --version "${ImagoRef_VERSION}" --install-location "/Applications" "${CMAKE_BINARY_DIR}/package/ImagoRef.pkg"
            COMMAND ${PRODUCTBUILD_EXECUTABLE} --package "${CMAKE_BINARY_DIR}/package/ImagoRef.pkg" "${CMAKE_BINARY_DIR}/ImagoRef_v${ImagoRef_VERSION}_macOS.pkg"
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/package"
            DEPENDS ImagoRef
            VERBATIM
        )
    endif()
endif()
