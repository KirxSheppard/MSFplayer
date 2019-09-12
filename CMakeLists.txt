cmake_minimum_required(VERSION 3.10)

###############################################################################
# PROJECT NAME AND VERSION
###############################################################################
project(VideoDecodeFFMPEG)
set(APPLICATION_NAME ${PROJECT_NAME})
set(APPLICATION_VERSION "1.2.0")


###############################################################################
# CMAKE PARAMS
###############################################################################
set(CMAKE_LINK_DEPENDS_NO_SHARED ON) # This is only for libraries, does nothing for executables - don't re-link all files when a library was changed
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

###############################################################################
# FILES
###############################################################################
file(GLOB PROJECT_FILES
    "source/*.cpp"
    "header/*.hpp"
    "header/*.h"
    "source/*.ui"
    "source/*.qrc"
)

add_definitions(-DQT_MESSAGELOGCONTEXT)

if(APPLE)
    set(MACOSX_BUNDLE_BUNDLE_VERSION "1")
    set(MACOSX_BUNDLE_ICON_FILE "icon.icns")
    set(MACOSX_BUNDLE_BUNDLE_NAME "MSF Player")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${APPLICATION_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2019  Kamil Janko")

    set(ICNS_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.icns")
    set_source_files_properties(${ICNS_FILE_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
elseif(WIN32)
    # set RC file
    list(APPEND PROJECT_FILES "icons.rc")
endif()

add_executable(${PROJECT_NAME}
    WIN32
    MACOSX_BUNDLE ${ICNS_FILE_PATH}
    ${PROJECT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "MSF Player")

target_include_directories(${PROJECT_NAME}
    PRIVATE
    "source"
    "header"
    "forms"
)

###############################################################################
# LIBS AND LINKING
###############################################################################
find_package(Qt5 COMPONENTS Widgets Core REQUIRED)

target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
)

if(APPLE)
    target_link_libraries(${PROJECT_NAME}
        "/usr/local/lib/libavcodec.dylib"
        "/usr/local/lib/libavutil.dylib"
        "/usr/local/lib/libavformat.dylib"
        "/usr/local/lib/libswscale.dylib"
    )
    target_include_directories(${PROJECT_NAME}
        PRIVATE
        "/usr/local/include"
    )
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME}
        "C:/Users/Sheppard/Downloads/ffmpeg-4.1.3-win64-dev/ffmpeg-4.1.3-win64-dev/lib/avcodec.lib"
        "C:/Users/Sheppard/Downloads/ffmpeg-4.1.3-win64-dev/ffmpeg-4.1.3-win64-dev/lib/avutil.lib"
        "C:/Users/Sheppard/Downloads/ffmpeg-4.1.3-win64-dev/ffmpeg-4.1.3-win64-dev/lib/avformat.lib"
        "C:/Users/Sheppard/Downloads/ffmpeg-4.1.3-win64-dev/ffmpeg-4.1.3-win64-dev/lib/swscale.lib"
    )
    target_include_directories(${PROJECT_NAME}
        PRIVATE
        "C:/Users/Sheppard/Downloads/ffmpeg-4.1.3-win64-dev/ffmpeg-4.1.3-win64-dev/include"
    )
else()
    target_link_libraries(${PROJECT_NAME}
        "libavcodec.so"
        "libavutil.so"
        "libavformat.so"
        "libswscale.so"
    )
endif()

###############################################################################
# INSTALL
###############################################################################
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/BundleInfo.plist.in")
    install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})

    set(QT_LIBS_DIR "${Qt5Core_DIR}/../..")
    set(QT_PLUGINS_DIR "${QT_LIBS_DIR}/../plugins")
    set(MAC_BUNDLE_PATH "${CMAKE_INSTALL_PREFIX}/MSF Player.app")
    install(FILES
        "${QT_PLUGINS_DIR}/platforms/libqcocoa.dylib"
        DESTINATION "${MAC_BUNDLE_PATH}/Contents/plugins/platforms")
    install(FILES
        "${QT_PLUGINS_DIR}/styles/libqmacstyle.dylib"
        DESTINATION "${MAC_BUNDLE_PATH}/Contents/plugins/styles")
    install(DIRECTORY
        "${QT_PLUGINS_DIR}/imageformats"
        DESTINATION "${MAC_BUNDLE_PATH}/Contents/plugins"
        PATTERN "*_debug.dylib" EXCLUDE
        PATTERN "*.dSYM" EXCLUDE)
    install(CODE "
        include(BundleUtilities)
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        list(APPEND DIRS /usr/local/lib ${QT_LIBS_DIR})
        file(GLOB_RECURSE QT_PLUGINS
            \"${MAC_BUNDLE_PATH}/Contents/plugins/*.dylib\")
        file(WRITE \"${MAC_BUNDLE_PATH}/Contents/Resources/qt.conf\")
        fixup_bundle(\"${MAC_BUNDLE_PATH}\" \"\${QT_PLUGINS}\" \"\${DIRS}\")
    ")
endif()
