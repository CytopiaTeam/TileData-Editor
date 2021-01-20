cmake_minimum_required(VERSION 3.10)

project(TileDataEditor)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(FORMS
        src/TileDataUI.ui
        src/TileSetDataUI.ui)

set(SOURCES
        src/main.cxx
        src/TileDataUI.cxx
        src/TileDataContainer.cxx)

set(HEADERS
        src/helpers.hxx
        src/TileDataContainer.hxx
        src/TileDataUI.hxx
        Cytopia/src/engine/basics/tileData.hxx
        Cytopia/src/engine/common/enums.hxx)

add_executable(TileDataEditor
        ${FORMS}
        ${HEADERS}
        ${SOURCES})

target_include_directories(TileDataEditor PUBLIC ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/Cytopia/src/engine/common
        ${CMAKE_SOURCE_DIR}/Cytopia/external/header_only)
target_link_libraries(TileDataEditor Qt5::Widgets)
