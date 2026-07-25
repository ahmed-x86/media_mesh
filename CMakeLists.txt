cmake_minimum_required(VERSION 3.16)
project(media_mesh VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# تم تغيير الاسم هنا إلى media_mesh
add_executable(media_mesh main.cpp)

# وتم تغيير الاسم هنا أيضاً
target_link_libraries(media_mesh PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)