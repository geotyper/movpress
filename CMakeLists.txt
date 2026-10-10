cmake_minimum_required(VERSION 3.21)

project(MovPress VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)

qt_add_executable(MovPress MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

target_link_libraries(MovPress PRIVATE Qt6::Widgets)

set_target_properties(MovPress PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.geotyper.movpress
    MACOSX_BUNDLE_BUNDLE_NAME "MovPress"
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
)

install(TARGETS MovPress
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
