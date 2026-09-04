cmake_minimum_required(VERSION 3.16)
project(kith LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Detect Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

message(STATUS "Compiling with Qt${QT_VERSION_MAJOR}")

add_executable(kith
  src/main.cpp
  src/CalendarWidget.cpp
  src/CalendarWidget.h
  resources/resources.qrc
)

target_link_libraries(kith PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Install the binary at /usr/local/bin
install(TARGETS kith RUNTIME DESTINATION bin)
