# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("player")
set(HOME_BASE_REL "../../../../../")
get_filename_component(HOME_BASE ${HOME_BASE_REL} REALPATH)
message(${HOME_BASE})
set(YVRTC_THIRD_DIR ${HOME_BASE}/thirdparty/lib/android/${CMAKE_ANDROID_ARCH_ABI})
set(YVRTC_LIB_DIR ${HOME_BASE}/bin/lib_android/${CMAKE_ANDROID_ARCH_ABI})
include_directories(${HOME_BASE}/include)
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        player

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)


find_library(camera-lib camera2ndk)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.


target_link_libraries( # Specifies the target library.
        player
        ${camera-lib}
        -Wl,--start-group
        ${YVRTC_LIB_DIR}/libyvrtc.a  ${YVRTC_LIB_DIR}/libyvrtccore.a
        ${YVRTC_THIRD_DIR}/libsrtp2.a ${YVRTC_THIRD_DIR}/libssl.a ${YVRTC_THIRD_DIR}/libcrypto.a
        ${YVRTC_THIRD_DIR}/libspeexdsp.a ${YVRTC_THIRD_DIR}/libyuv.a ${YVRTC_THIRD_DIR}/libusrsctp.a
        ${YVRTC_THIRD_DIR}/libyangh264decoder.a
        ${YVRTC_THIRD_DIR}/libopus.a
        ${YVRTC_THIRD_DIR}/libopenh264.a
        -Wl,--end-group
        # Links the target library to the log library
        # included in the NDK.
        EGL GLESv2 OpenSLES android log mediandk)