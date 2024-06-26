# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${PROJECT_NAME}.
project("mathias")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.

set(OpenCV_DIR C:/Users/Simon/AndroidStudioProjects/OpenCV/sdk/native/jni)

find_package(OpenCV REQUIRED)
add_library(${PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        native-lib.cpp
        CV_Manager.cpp
        Native_Camera.cpp
        Image_Reader.cpp
        Socket_client.cpp
        Encoder.cpp
        )

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${PROJECT_NAME}
        # List libraries link to the target library
        android
        ${OpenCV_LIBS}
        log
        ${log-lib}
        camera2ndk
        )

# Introduce preprocessor variables to keep paths of asset files
set(FACE_DETECTION_CASCADE "${PROJECT_SOURCE_DIR}/assets/haarcascade_frontalface_default.xml")
set(FACE_DETECTION_CONFIGURATION "${PROJECT_SOURCE_DIR}/assets/deploy.prototxt")
set(FACE_DETECTION_WEIGHTS "${PROJECT_SOURCE_DIR}/assets/res10_300x300_ssd_iter_140000_fp16.caffemodel")
target_compile_definitions(${PROJECT_NAME} PRIVATE  FACE_DETECTION_CASCADE="${FACE_DETECTION_CASCADE}")
target_compile_definitions(${PROJECT_NAME} PRIVATE  FACE_DETECTION_CONFIGURATION="${FACE_DETECTION_CONFIGURATION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE  FACE_DETECTION_WEIGHTS="${FACE_DETECTION_WEIGHTS}")