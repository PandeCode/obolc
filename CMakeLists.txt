cmake_minimum_required(VERSION 3.16)
project(obolc VERSION 0.1 LANGUAGES CXX)

# C++ settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt auto features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Prefer Clang if found
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

# Optional: mold linker
if(MOLD_LINKER AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Using Mold linker for Debug build: ${MOLD_LINKER}")
  add_link_options("-fuse-ld=mold")
endif()

# Qt + LayerShellQt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets DBus)
find_package(LayerShellQt REQUIRED)

# Source discovery
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.hpp)

# Executable
add_executable(obolc ${SOURCES})

target_link_libraries(obolc
  Qt6::Core Qt6::Widgets Qt6::DBus LayerShellQt::Interface
)

target_include_directories(obolc PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Precompiled headers
target_precompile_headers(obolc PRIVATE
  <QApplication> <QDateTime> <QFile> <QHBoxLayout> <QJsonArray>
  <QJsonDocument> <QJsonObject> <QLabel> <QMessageBox> <QProcess>
  <QPushButton> <QRegularExpression> <QScreen> <QTextStream>
  <QTimer> <QWidget> <cstdint>
)

# SCSS compilation (style.scss -> assets/style.css)
set(SCSS_INPUT "${CMAKE_SOURCE_DIR}/style.scss")
set(CSS_OUTPUT "${CMAKE_SOURCE_DIR}/assets/style.css")

file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/assets")

add_custom_command(
  OUTPUT "${CSS_OUTPUT}"
  COMMAND sass --no-source-map "${SCSS_INPUT}" "${CSS_OUTPUT}"
  DEPENDS "${SCSS_INPUT}"
  COMMENT "Compiling SCSS to CSS"
  VERBATIM
)

add_custom_target(compile_scss ALL DEPENDS "${CSS_OUTPUT}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_custom_command(TARGET obolc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/assets
      $<TARGET_FILE_DIR:obolc>/assets
  )
endif()

install(TARGETS obolc DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/
        DESTINATION share/obolc/assets)
