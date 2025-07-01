cmake_minimum_required(VERSION 3.16)
project(
  obolc
  VERSION 0.1
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

find_program(CLANG_COMPILER clang++)
if(CLANG_COMPILER)
    set(CMAKE_CXX_COMPILER ${CLANG_COMPILER})
    message(STATUS "Using Clang compiler: ${CLANG_COMPILER}")

    add_compile_options(
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
        -Wcast-align -Wunused -Wold-style-cast -Wnon-virtual-dtor
    )
else()
    message(STATUS "Clang not found, using default compiler")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(MOLD_LINKER AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Using Mold linker for Debug build: ${MOLD_LINKER}")
    add_link_options("-fuse-ld=mold")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(LayerShellQt REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.hpp)

add_executable(obolc ${SOURCES})

target_link_libraries(obolc
    Qt6::Core Qt6::Widgets LayerShellQt::Interface)

target_include_directories(obolc PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_precompile_headers(
  obolc
  PRIVATE
  <QApplication>
  <QDateTime>
  <QFile>
  <QHBoxLayout>
  <QJsonArray>
  <QJsonDocument>
  <QJsonObject>
  <QLabel>
  <QMessageBox>
  <QProcess>
  <QPushButton>
  <QRegularExpression>
  <QScreen>
  <QTextStream>
  <QTimer>
  <QWidget>
  <cstdint>
)

install(TARGETS obolc DESTINATION bin)

# set(CMAKE_VERBOSE_MAKEFILE ON)
