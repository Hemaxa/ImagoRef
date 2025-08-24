# Определение минимальной версии CMake и проекта
cmake_minimum_required(VERSION 3.21.1)
project(ImagoRef VERSION 1.0 LANGUAGES CXX)

# Определение минимальной версии macOS
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version" FORCE)

set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS")

# Определение используемых пакетов
find_package(Qt5 COMPONENTS Core Gui Widgets Svg REQUIRED)

# Определение cтандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включение Qt-инструментов
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Отключение сжатия ресурсов
set(CMAKE_AUTORCC_OPTIONS "--no-compress")

# Создание переменной для src директории
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Создание переменной со списком исходных файлов
set(IMAGO_SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/app/mainwindow.cpp
    ${SRC_DIR}/app/undocommands.cpp
    ${SRC_DIR}/widgets/floatingtoolbar.cpp
    ${SRC_DIR}/widgets/settingsdialog.cpp
    ${SRC_DIR}/graphics/canvasview.cpp
    ${SRC_DIR}/graphics/imageitem.cpp
    ${SRC_DIR}/../res/resources.qrc
)

# Создание исполняемого файла
add_executable(ImagoRef MACOSX_BUNDLE ${IMAGO_SOURCES})

# Установка свойств для macOS
set_target_properties(ImagoRef PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
)

# Директории с заголовочными файлами
target_include_directories(ImagoRef PRIVATE
    ${SRC_DIR}/app
    ${SRC_DIR}/widgets
    ${SRC_DIR}/graphics
)

# Обозначение включенных библиотеки
target_link_libraries(ImagoRef PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Svg
)

# Развертывание для macOS
find_program(MACDEPLOYQT_EXECUTABLE macdeployqt)

add_custom_command(TARGET ImagoRef POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/res/app_icon/icon.icns"
        "$<TARGET_BUNDLE_DIR:ImagoRef>/Contents/Resources/icon.icns"
    COMMAND ${MACDEPLOYQT_EXECUTABLE}
        "$<TARGET_BUNDLE_DIR:ImagoRef>"
        -verbose=1
    COMMAND codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:ImagoRef>"
)

# Создание .pkg для macOS
find_program(PKGBUILD_EXECUTABLE pkgbuild)
find_program(PRODUCTBUILD_EXECUTABLE productbuild)

if(PKGBUILD_EXECUTABLE AND PRODUCTBUILD_EXECUTABLE)
    set(PACKAGE_DIR "${CMAKE_BINARY_DIR}/package")
    add_custom_target(package
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PACKAGE_DIR}/root"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_BUNDLE_DIR:ImagoRef>" "${PACKAGE_DIR}/root/ImagoRef.app"
        COMMAND ${PKGBUILD_EXECUTABLE} --root "${PACKAGE_DIR}/root" --identifier "com.imago.ref" --version "${ImagoRef_VERSION}" --install-location "/Applications" "${PACKAGE_DIR}/ImagoRef.pkg"
        COMMAND ${PRODUCTBUILD_EXECUTABLE} --package "${PACKAGE_DIR}/ImagoRef.pkg" "${CMAKE_BINARY_DIR}/ImagoRef_v${ImagoRef_VERSION}_macOS.pkg"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${PACKAGE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E echo "Package created: ${CMAKE_BINARY_DIR}/ImagoRef_v${ImagoRef_VERSION}_macOS.pkg"
        DEPENDS ImagoRef
        COMMENT "Creating package ImagoRef-${ImagoRef_VERSION}.pkg"
        VERBATIM
    )
else()
    message(WARNING "pkgbuild/productbuild utilities not found. 'package' target is not available.")
endif()
