#определение минимальной версии CMake и проекта
cmake_minimum_required(VERSION 3.21.1)
project(ImagoRef VERSION 1.0 LANGUAGES CXX)

#определение минимальной версии macOS (флаги для компилятора и линковщика)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version" FORCE)
add_compile_options(-mmacosx-version-min=10.13)
add_link_options(-mmacosx-version-min=10.13)

#определение поддерживаемых архитектур (x86_64 и arm64)
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for macOS")

#определение используемых пакетов
find_package(Qt5 COMPONENTS Core Gui Widgets Svg REQUIRED)

#определение cтандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#включение Qt-инструментов
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#отключение сжатия ресурсов
set(CMAKE_AUTORCC_OPTIONS "--no-compress")

#создание переменной для src директории
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

#создание исполняемого файла для Windows и macOS
add_executable(ImagoRef WIN32 MACOSX_BUNDLE)

#установка свойств для macOS
if(APPLE)
    set_target_properties(ImagoRef PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )
endif()

#установка свойств для Windows
if(WIN32)
    target_sources(ImagoRef PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc
    )
endif()

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

#обозначение включенных библиотеки
target_link_libraries(ImagoRef PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Svg
)

#развертывание для macOS
if(APPLE)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt)

    #добавление иконки, запуск macdeployqt, подписывание приложения
    add_custom_command(TARGET ImagoRef POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/res/app_icon/icon.icns"
            "$<TARGET_BUNDLE_DIR:ImagoRef>/Contents/Resources/icon.icns"

        COMMAND ${MACDEPLOYQT_EXECUTABLE}
            "$<TARGET_BUNDLE_DIR:ImagoRef>"
            -verbose=1

        COMMAND codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:ImagoRef>"
    )
endif()

#создание .pkg для macOS
if(APPLE)
    #поиск утилит для создания установочного пакета
    find_program(PKGBUILD_EXECUTABLE pkgbuild)
    find_program(PRODUCTBUILD_EXECUTABLE productbuild)

    if(PKGBUILD_EXECUTABLE AND PRODUCTBUILD_EXECUTABLE)
        #создание каталога для временных файлов
        set(PACKAGE_DIR "${CMAKE_BINARY_DIR}/package")

        #создание пакета с установкой в Applications
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
endif()

#развертывание для Windows
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR WIN32)
    get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)

    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ImagoRef POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                "$<TARGET_FILE_DIR:ImagoRef>"
                --no-compiler-runtime
                --no-system-d3d-compiler
                --no-opengl-sw
                --no-angle
                --verbose 1
            COMMENT "Deploying Qt libraries for Windows..."
        )
    else()
        message(WARNING "windeployqt not found. Windows deployment will not work.")
    endif()

    include(InstallRequiredSystemLibraries)
    set(CPACK_PACKAGE_NAME "ImagoRef")
    set(CPACK_PACKAGE_VENDOR "Yashin Sergey")
    set(CPACK_PACKAGE_VERSION "${ImagoRef_VERSION}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "ImagoRef")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/app_icon/icon.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/res/app_icon/icon.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "icon.ico")
    set(CPACK_NSIS_DISPLAY_NAME "ImagoRef")
    set(CPACK_NSIS_PACKAGE_NAME "ImagoRef")
    set(CPACK_NSIS_MODIFY_PATH ON)

    set(CPACK_GENERATOR "NSIS")

    set(CPACK_NSIS_CREATE_ICONS_EXTRA "
            CreateShortCut '$DESKTOP\\\\ImagoRef.lnk' '$INSTDIR\\\\ImagoRef.exe'
        ")

    set(CPACK_NSIS_DELETE_ICONS_EXTRA "
        Delete '$DESKTOP\\\\ImagoRef.lnk'
    ")

    install(TARGETS ImagoRef
        RUNTIME DESTINATION "."
        BUNDLE DESTINATION "."
        LIBRARY DESTINATION "."
    )

    include(CPack)
endif()
