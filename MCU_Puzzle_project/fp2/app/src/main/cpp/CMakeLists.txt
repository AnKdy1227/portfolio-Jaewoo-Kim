# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("fp2")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(JNIDriver
        SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        JNIDriver.c)

add_library(GLES_mali STATIC IMPORTED)

add_library( # Sets the name of the library.
        OpenCLDriver

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        OpenCLDriver.c )

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp )
# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
# Links the JNIDriver library with required Android system libraries.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log )

find_library( # Sets the name of the path variable.
        jnigraphics-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        jnigraphics )

target_link_libraries( # Specifies the target library.
        native-lib

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} )
target_link_libraries(JNIDriver
         # Android NDK 관련 라이브러리
        ${log-lib}     # Android 로그 라이브러리
)
target_link_libraries( # Specifies the target library.
        OpenCLDriver

        # Links the target library to the log library
        # included in the NDK.
        C:/Users/andykim02/Desktop/vcodefolder/MP/lib/libGLES_mali.so ${log-lib} ${jnigraphics-lib} )

target_include_directories(native-lib PRIVATE src/main.cpp)
