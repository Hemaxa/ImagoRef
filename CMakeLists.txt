#определение минимальной версии CMake
cmake_minimum_required(VERSION 3.21.1)

#определение версии проекта
project(ImagoRef VERSION 1.0 LANGUAGES CXX)

#определение cтандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#определение используемых пакетов (Qt Quick вместо Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2 Qml Svg)

#включение Qt-автоматизаций
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#создание переменной для src директории
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

#создание исполняемого файла
qt_add_executable(ImagoRef
    ${SRC_DIR}/main.cpp
)

target_sources(ImagoRef PRIVATE
    ${SRC_DIR}/controllers/BoardController.h
    ${SRC_DIR}/controllers/BoardController.cpp
    ${SRC_DIR}/controllers/StackController.h
    ${SRC_DIR}/controllers/StackController.cpp
    ${SRC_DIR}/controllers/FileController.h
    ${SRC_DIR}/controllers/FileController.cpp
    ${SRC_DIR}/controllers/SelectionController.h
    ${SRC_DIR}/controllers/SelectionController.cpp
    ${SRC_DIR}/controllers/ClipboardController.h
    ${SRC_DIR}/controllers/ClipboardController.cpp
    ${SRC_DIR}/controllers/ToolController.h
    ${SRC_DIR}/controllers/ToolController.cpp
    ${SRC_DIR}/controllers/UpscaleController.h
    ${SRC_DIR}/controllers/UpscaleController.cpp

    ${SRC_DIR}/managers/SettingsManager.h
    ${SRC_DIR}/managers/SettingsManager.cpp
    ${SRC_DIR}/managers/ThemesManager.h
    ${SRC_DIR}/managers/ThemesManager.cpp
    ${SRC_DIR}/managers/ModelsManager.h
    ${SRC_DIR}/managers/ModelsManager.cpp

    ${SRC_DIR}/models/ImageModel.h
    ${SRC_DIR}/models/ImageModel.cpp
    ${SRC_DIR}/models/ImageProvider.h
    ${SRC_DIR}/models/ImageProvider.cpp
)

qt_add_qml_module(ImagoRef
    URI ImagoRef
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml

        src/qml/windows/WelcomeWindow.qml
        src/qml/windows/MainWindow.qml
        src/qml/windows/SettingsWindow.qml

        src/qml/components/CanvasView.qml
        src/qml/components/ImageItem.qml
        src/qml/components/FloatingToolbar.qml
        src/qml/components/ToolbarButton.qml
        src/qml/components/ResizeOverlay.qml
        src/qml/components/CropOverlay.qml
)

# Копирование папки с темами в директорию сборки
add_custom_command(TARGET ImagoRef POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/res/themes"
    "$<TARGET_FILE_DIR:ImagoRef>/themes"
)

#директории с заголовочными файлами
target_include_directories(ImagoRef PRIVATE
    ${SRC_DIR}/controllers
    ${SRC_DIR}/managers
    ${SRC_DIR}/models
    ${ncnn_SOURCE_DIR}/src
    ${ncnn_BINARY_DIR}/src
)

# ====================================================================
# QUAZIP
# ====================================================================
find_package(ZLIB REQUIRED)

set(QUAZIP_ENABLE_TESTS OFF CACHE BOOL "Build QuaZip tests" FORCE)

include(FetchContent)
FetchContent_Declare(
    quazip
    GIT_REPOSITORY https://github.com/stachenov/quazip.git
    GIT_TAG        v1.4
    GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(quazip)

target_link_libraries(ImagoRef PRIVATE QuaZip::QuaZip)

# ====================================================================
# NCNN
# ====================================================================
set(NCNN_VULKAN OFF CACHE BOOL "build vulkan compute support" FORCE)
set(NCNN_BUILD_BENCHMARK OFF CACHE BOOL "build benchmark" FORCE)
set(NCNN_BUILD_EXAMPLES OFF CACHE BOOL "build examples" FORCE)
set(NCNN_BUILD_TESTS OFF CACHE BOOL "build tests" FORCE)
set(NCNN_BUILD_TOOLS OFF CACHE BOOL "build tools" FORCE)
set(NCNN_BUILD_EXAMPLES OFF CACHE BOOL "build examples" FORCE)
set(NCNN_INSTALL_MODULES OFF CACHE BOOL "install modules" FORCE)
set(NCNN_PIXEL ON CACHE BOOL "build with pixel conversions" FORCE)

FetchContent_Declare(
    ncnn
    GIT_REPOSITORY https://github.com/Tencent/ncnn.git
    GIT_TAG        20260113
    GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(ncnn)

target_link_libraries(ImagoRef PRIVATE ncnn)


# ====================================================================
# НАСТРОЙКИ ДЛЯ MACOS
# ====================================================================
if(APPLE)
    #поиск системных библиотек OpenGL (2D/3D графика) и ZLIB (сжатие данных)
    find_package(OpenGL REQUIRED)
    find_package(ZLIB REQUIRED)

    #подключение необходимых библиотек
    target_link_libraries(ImagoRef PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Qml
        Qt6::Gui
        Qt6::GuiPrivate
        Qt6::Svg

        OpenGL::GL
        ZLIB::ZLIB
        "-framework AppKit"
        "-framework Foundation"
        "-framework IOKit"
        "-framework CoreGraphics"
    )

    #задание минимальной версии macOS (macOS 11 Big Sur)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11" CACHE STRING "Minimum OS X deployment version" FORCE)

    #задание архитектуры процессора
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS")

    #задание свойств конечного файла (Info.plist.in)
    set_target_properties(ImagoRef PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/packs/Info.plist.in"
    )

    #поиск необходимых утилит для сборки проекта
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt)
    find_program(PKGBUILD_EXECUTABLE pkgbuild)
    find_program(PRODUCTBUILD_EXECUTABLE productbuild)

    #копирование иконки в .app
    add_custom_command(TARGET ImagoRef POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/res/app-icon/icon.icns" "$<TARGET_BUNDLE_DIR:ImagoRef>/Contents/Resources/icon.icns"
    )

    #создание и настройка .pkg
    if(PKGBUILD_EXECUTABLE AND PRODUCTBUILD_EXECUTABLE)
        add_custom_target(package
            #запуск macdeployqt
            COMMAND ${MACDEPLOYQT_EXECUTABLE} "$<TARGET_BUNDLE_DIR:ImagoRef>" -qmldir=${SRC_DIR}/qml -verbose=1

            #подпись приложения
            COMMAND codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:ImagoRef>"

            #создание временной директории
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/package/root/Applications"

            #перемещение .app во временную директорию
            COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_BUNDLE_DIR:ImagoRef>" "${CMAKE_BINARY_DIR}/package/root/Applications/ImagoRef.app"

            #упаковка в .pkg
            COMMAND ${PKGBUILD_EXECUTABLE}
                --root "${CMAKE_BINARY_DIR}/package/root"
                --identifier "com.imago.ref"
                --version "${ImagoRef_VERSION}"
                --install-location "/" "${CMAKE_BINARY_DIR}/package/ImagoRef.pkg"

            #настройка .pkg
            COMMAND ${PRODUCTBUILD_EXECUTABLE}
                --package "${CMAKE_BINARY_DIR}/package/ImagoRef.pkg" "${CMAKE_BINARY_DIR}/ImagoRef_v${ImagoRef_VERSION}_macOS.pkg"

            #удаление временной директории
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/package"

            DEPENDS ImagoRef
            VERBATIM
        )
    endif()
endif()

# ====================================================================
# НАСТРОЙКИ ДЛЯ WINDOWS
# ====================================================================
if(WIN32)
    #подключение необходимых библиотек
    target_link_libraries(ImagoRef PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Qml
        Qt6::Gui
        Qt6::GuiPrivate
        Qt6::Svg
    )

    #задание свойств конечного файла
    set_target_properties(ImagoRef PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    #добавление файла ресурсов (resources.rc)
    target_sources(ImagoRef PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc)

    #исполняемый файл помещается в /bin
    install(TARGETS ImagoRef DESTINATION bin)

    #копирование иконки в /bin
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/app_icon/icon.ico" DESTINATION bin)

    #создание и настройка .exe
    set(CPACK_PACKAGE_NAME "ImagoRef")
    set(CPACK_PACKAGE_VENDOR "Yashin Sergey")
    set(CPACK_PACKAGE_VERSION "${ImagoRef_VERSION}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "ImagoRef")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/app_icon/icon.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/res/app_icon/icon.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin/ImagoRef.exe" CACHE STRING "" FORCE)
    set(CPACK_NSIS_DISPLAY_NAME "ImagoRef")
    set(CPACK_NSIS_PACKAGE_NAME "ImagoRef ${ImagoRef_VERSION}")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/docs/LICENSE")
    set(CPACK_NSIS_MODIFY_PATH OFF)
    set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$DESKTOP\\\\ImagoRef.lnk' '$INSTDIR\\\\bin\\\\ImagoRef.exe'")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$DESKTOP\\\\ImagoRef.lnk'")
    set(CPACK_PACKAGE_FILE_NAME "ImagoRef_v${ImagoRef_VERSION}_windows")
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_INSTALLED_DIRECTORIES "${CMAKE_INSTALL_PREFIX};.")

    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
        "WriteRegStr HKCR '.iref' '' 'ImagoRef.Board' \\n \
         WriteRegStr HKCR 'ImagoRef.Board' '' 'ImagoRef доска' \\n \
         WriteRegStr HKCR 'ImagoRef.Board\\\\DefaultIcon' '' '$INSTDIR\\\\\\\\bin\\\\\\\\ImagoRef.exe,1' \\n \
         WriteRegStr HKCR 'ImagoRef.Board\\\\shell' '' 'open' \\n \
         WriteRegStr HKCR 'ImagoRef.Board\\\\shell\\\\open' '' 'Открыть в ImagoRef' \\n \
         WriteRegStr HKCR 'ImagoRef.Board\\\\shell\\\\open\\\\command' '' '\"$INSTDIR\\\\\\\\bin\\\\\\\\ImagoRef.exe\" \"%1\"' \\n "
    )

    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
        "DeleteRegKey HKCR '.iref' \\n \
         DeleteRegKey HKCR 'ImagoRef.Board' \\n "
    )
    include(CPack)
endif()

# ====================================================================
# НАСТРОЙКИ ДЛЯ LINUX
# ====================================================================
if(UNIX AND NOT APPLE)
    #подключение необходимых библиотек
    target_link_libraries(ImagoRef PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Qml
        Qt6::Gui
        Qt6::GuiPrivate
        Qt6::Svg
    )

    #регистрация приложения (Info.desktop.in)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Info.desktop.in"
        "${CMAKE_CURRENT_BINARY_DIR}/Info.desktop"
        COPYONLY
    )

    #исполняемый файл помещается в /bin
    install(TARGETS ImagoRef DESTINATION bin)

    #копирование Info.desktop.in в /applications
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/Info.desktop"
        DESTINATION share/applications
        RENAME ImagoRef.desktop
    )

    #копирование иконки приложения
    install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/app_icon/icon.png"
        DESTINATION share/icons/hicolor/256x256/apps
        RENAME ImagoRef.png
    )
endif()
