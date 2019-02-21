cmake_minimum_required(VERSION 3.13)
project(simple_demo)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(inc)
include_directories(src)

add_executable(simple_demo
        inc/App.h
        inc/BoundingBox.h
        inc/Camera.h
        inc/Experiment.h
        inc/FrameScaler.h
        inc/FrameScalerSampleApp.h
        inc/graphics_context_t.h
        inc/lodepng.h
        inc/LpGLEngine.h
        inc/MeshLoader.h
        inc/MLNativeWindow.h
        inc/ModelObj.h
        inc/Quad.h
        inc/ShaderUtils.h
        inc/Simplify.h
        inc/tinyobjloader.h
        src/FrameScalerSampleApp.cpp
        src/graphics_context_t.cpp
        src/lodepng.cpp
        src/LpGLEngine.cpp
        src/Main.cpp
        src/MeshLoader.cpp
        src/MLNativeWindow.cpp
        src/ModelObj.cpp
        src/Quad.cpp
        src/ShaderUtils.cpp
        )
