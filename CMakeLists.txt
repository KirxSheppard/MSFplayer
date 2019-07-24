cmake_minimum_required(VERSION 3.10)

###############################################################################
# PROJECT NAME AND VERSION
###############################################################################
project(VideoDecodeFFMPEG)
set(APPLICATION_NAME ${PROJECT_NAME})
#set(APPLICATION_VERSION "1.0.0");


###############################################################################
# CMAKE PARAMS
###############################################################################
set(CMAKE_LINK_DEPENDS_NO_SHARED ON) # This is only for libraries, does nothing for executables - don't re-link all files when a library was changed
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Prevent "Default" builds
if(NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "Please choose build type")
endif()

###############################################################################
# FILES
###############################################################################
file(GLOB PROJECT_FILES
    "source/*.cpp"
    "source/*.hpp"
    "source/*.h"
    "source/*.ui"
    "source/*.qrc"
)

add_definitions(-DQT_MESSAGELOGCONTEXT)

add_executable(${PROJECT_NAME} WIN32 ${PROJECT_FILES})

###############################################################################
# LIBS AND LINKING
###############################################################################
find_package(Qt5 COMPONENTS Widgets Core REQUIRED)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    "C:/Users/Sheppard/Downloads/ffmpeg-4.1.3-win64-dev/ffmpeg-4.1.3-win64-dev/include"
    "source"
    "forms"
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
    "C:/Users/Sheppard/Downloads/ffmpeg-4.1.3-win64-dev/ffmpeg-4.1.3-win64-dev/lib/avcodec.lib"
    "C:/Users/Sheppard/Downloads/ffmpeg-4.1.3-win64-dev/ffmpeg-4.1.3-win64-dev/lib/avutil.lib"
    "C:/Users/Sheppard/Downloads/ffmpeg-4.1.3-win64-dev/ffmpeg-4.1.3-win64-dev/lib/avformat.lib"
    "C:/Users/Sheppard/Downloads/ffmpeg-4.1.3-win64-dev/ffmpeg-4.1.3-win64-dev/lib/swscale.lib"
)
