
cmake_minimum_required(VERSION 3.16)

project(Camera
    VERSION 1.0.0
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ==================== 1. 查找依赖 ====================

find_package(OpenCV 4 REQUIRED COMPONENTS core imgproc)

find_path(MVS_INCLUDE_DIR
    NAMES MvCameraControl.h
    PATHS
        /opt/MVS/include
)

find_library(MVS_LIBRARY
    NAMES MvCameraControl
    PATHS
        /opt/MVS_Runtime/MVS/lib/64
)

if(NOT MVS_INCLUDE_DIR OR NOT MVS_LIBRARY)
    message(FATAL_ERROR
        "MVS SDK not found. Please check MVS installation."
    )
endif()

get_filename_component(
    MVS_LIBRARY_DIR
    "${MVS_LIBRARY}"
    DIRECTORY
)

# ==================== 2. 创建 Camera 动态库 ====================

add_library(Camera SHARED
    src/Camera.cpp
)

# 安装后对外提供 Camera::Camera 这个目标名称。
add_library(Camera::Camera ALIAS Camera)

target_compile_features(Camera
    PRIVATE
        cxx_std_14
)

# ==================== 3. 配置头文件目录 ====================

target_include_directories(Camera
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>

    PRIVATE
        ${MVS_INCLUDE_DIR}
)

# ==================== 4. 链接依赖 ====================

target_link_libraries(Camera
    PUBLIC
        ${OpenCV_LIBS}

    PRIVATE
        ${MVS_LIBRARY}
)

# ==================== 5. 动态库属性 ====================

set_target_properties(Camera PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}

    BUILD_RPATH "${MVS_LIBRARY_DIR}"
    INSTALL_RPATH "${MVS_LIBRARY_DIR}"
)

# ==================== 6. 安装动态库和头文件 ====================

install(TARGETS Camera
    EXPORT CameraTargets

    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    include/Camera.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# ==================== 7. 安装 CMake 导出目标 ====================

install(EXPORT CameraTargets
    FILE CameraTargets.cmake
    NAMESPACE Camera::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Camera
)

# ==================== 8. 生成 CameraConfig.cmake ====================

configure_package_config_file(
    cmake/CameraConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CameraConfig.cmake

    INSTALL_DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/Camera
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/CameraConfigVersion.cmake

    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CameraConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CameraConfigVersion.cmake

    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Camera
)

# ==================== 9. 可选测试程序 ====================

option(BUILD_CAMERA_TESTS
    "Build Camera test program"
    OFF
)

if(BUILD_CAMERA_TESTS)
    add_executable(camera_test
        tests/camera_test.cpp
    )

    target_compile_features(camera_test
        PRIVATE
            cxx_std_14
    )

    target_link_libraries(camera_test
        PRIVATE
            Camera
    )
endif()

# ==================== 10. 输出依赖信息 ====================

message(STATUS "Camera version: ${PROJECT_VERSION}")
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "MVS include: ${MVS_INCLUDE_DIR}")
message(STATUS "MVS library: ${MVS_LIBRARY}")
