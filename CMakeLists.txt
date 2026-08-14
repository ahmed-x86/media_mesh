cmake_minimum_required(VERSION 3.16)
project(MediaMesh VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(PkgConfig REQUIRED)

# نحتاج هذه فقط لاكتشاف العتاد في MochaHwDialog
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavutil)

add_executable(MediaMesh
    main.cpp
    HomeWindow.cpp HomeWindow.h
    ModernConverterWindow.cpp ModernConverterWindow.h
    RippleButton.cpp RippleButton.h
    FileInfoBox.cpp FileInfoBox.h
    MochaMsgBox.h
    MochaHwDialog.h
    Constants.h
)

target_link_libraries(MediaMesh PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    PkgConfig::FFMPEG
)