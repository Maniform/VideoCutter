cmake_minimum_required(VERSION 3.16)

project(VideoCutter VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(WIN32_RESOURCES VideoCutter.rc)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui

        ${WIN32_RESOURCES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    set(app_icon_resource_windows ${WIN32_RESOURCES})
    qt_add_executable(VideoCutter
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${app_icon_resource_windows}

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET VideoCutter APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(VideoCutter SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(VideoCutter
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(VideoCutter PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.VideoCutter)
endif()
set_target_properties(VideoCutter PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS VideoCutter
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(VideoCutter)
endif()

add_custom_command(TARGET VideoCutter POST_BUILD
    COMMAND windeployqt --release $<TARGET_FILE:VideoCutter>
    COMMENT "Running windeployqt..."
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/ DESTINATION bin)
install(FILES ffmpeg.exe DESTINATION bin)

set(CPACK_PACKAGE_NAME "VideoCutter")
set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
#set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_SYSTEM_NAME}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_PACKAGE_EXECUTABLES "VideoCutter" "Video Cutter")
include(CPack)
