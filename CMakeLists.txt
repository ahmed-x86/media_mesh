cmake_minimum_required(VERSION 3.16)
project(MediaMesh VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# تفعيل أدوات Qt التلقائية
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# البحث عن مكتبات Qt وتضمين DBus
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets DBus)

# تجميع ملفات المشروع
set(PROJECT_SOURCES
    main.cpp
    RippleButton.cpp RippleButton.h
    FileInfoBox.cpp FileInfoBox.h
    ModernConverterWindow.cpp ModernConverterWindow.h
    HomeWindow.cpp HomeWindow.h
    Constants.h
    MochaMsgBox.h
)

# إنشاء الملف التنفيذي
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# ربط المكتبات
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
)