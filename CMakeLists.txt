cmake_minimum_required(VERSION 3.21)

project(MovPress VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

qt_add_executable(MovPress MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mediautils.cpp
    src/mediautils.h
    src/cropoverlay.cpp
    src/cropoverlay.h
)

target_link_libraries(MovPress PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)

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

if(APPLE)
    find_program(FFMPEG_EXECUTABLE ffmpeg)
    find_program(FFPROBE_EXECUTABLE ffprobe)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(NOT Python3_EXECUTABLE)
        find_program(Python3_EXECUTABLE python3)
    endif()
    if(FFMPEG_EXECUTABLE AND FFPROBE_EXECUTABLE AND Python3_EXECUTABLE)
        add_custom_command(TARGET MovPress POST_BUILD
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/cmake/bundle_ffmpeg.py"
                    "$<TARGET_FILE_DIR:MovPress>"
                    "${FFMPEG_EXECUTABLE}"
                    "${FFPROBE_EXECUTABLE}"
            COMMENT "Copying ffmpeg and ffprobe into the app bundle"
            VERBATIM)
    else()
        message(WARNING "ffmpeg, ffprobe, or python3 was not found. The bundle will look for them on PATH at runtime.")
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
    qt_add_executable(MovPressTests
        tests/test_mediautils.cpp
        src/mediautils.cpp
        src/mediautils.h
    )
    target_include_directories(MovPressTests PRIVATE src)
    target_link_libraries(MovPressTests PRIVATE Qt6::Test)
    add_test(NAME MovPressTests COMMAND MovPressTests)
endif()
