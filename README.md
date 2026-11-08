# Camera

一个基于 **海康机器人 MVS SDK** 和 **OpenCV** 的 C++ 相机采集库。

本项目对 MVS SDK 进行封装，提供简洁的相机初始化、图像采集和资源管理接口，方便在不同的 C++ 视觉项目中重复使用。

## 主要功能

- **RAII 资源管理**：通过 C++ 对象生命周期自动管理相机资源，对象析构时自动执行资源释放。
- **一键启动**：通过 `Camera::start()` 完成 SDK 初始化、设备枚举、设备打开、参数配置和图像采集启动。
- **OpenCV 图像接口**：将相机采集的 Bayer 图像转换为 OpenCV `cv::Mat` 格式的 BGR 图像。
- **错误信息查询**：通过 `Camera::last_error()` 获取最近一次操作失败的错误信息。
- **独立 CMake 库**：支持源码接入和安装包接入，便于在不同项目中复用。

## 环境与依赖

### 开发环境

- Ubuntu Linux
- CMake 3.16 或更高版本
- 支持 C++14 的编译器
- OpenCV 4.x（需要 `core` 和 `imgproc` 模块）
- 海康机器人 MVS SDK 和 Runtime

### MVS SDK

本项目依赖海康机器人官方 MVS SDK。

MVS SDK 不包含在本仓库中，请自行从海康机器人官方渠道获取并安装。

当前 `CMakeLists.txt` 默认在以下位置查找 MVS：

| 依赖 | 默认路径 |
|---|---|
| MVS 头文件 | `/opt/MVS/include` |
| MVS 动态库 | `/opt/MVS_Runtime/MVS/lib/64` |

如果 MVS 安装位置不同，需要相应修改项目 `CMakeLists.txt` 中的 `find_path()` 和 `find_library()` 搜索路径。

注意：使用本库的计算机也必须安装兼容的 MVS Runtime，否则程序无法正常加载相机 SDK。

## 编译动态库

克隆仓库：

```bash
git clone <你的 GitHub 仓库地址>
cd Camera
```

配置项目：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

编译动态库：

```bash
cmake --build build --parallel
```

编译成功后，动态库位于：

```text
build/libCamera.so
```

## 快速使用

### 1. 引入头文件

```cpp
#include <Camera.h>

#include <iostream>
#include <opencv2/core.hpp>

int main()
{
    // 创建相机对象。
    Camera camera;

    // 一键启动相机。
    if (!camera.start())
    {
        std::cerr << "Camera startup failed: "
                  << camera.last_error() << std::endl;

        return 1;
    }

    // 获取一帧图像。
    cv::Mat frame;

    if (!camera.get_frame(frame))
    {
        std::cerr << "Frame acquisition failed: "
                  << camera.last_error() << std::endl;

        return 1;
    }

    std::cout << "Image size: "
              << frame.cols << " x " << frame.rows
              << std::endl;

    // 离开作用域时，Camera 对象自动析构并释放资源。
    return 0;
}
```

### 2. 主要接口

| 接口 | 功能 |
|---|---|
| `Camera()` | 创建相机对象 |
| `start(int device_id = 0)` | 一键启动相机，默认使用第一个设备 |
| `get_frame(cv::Mat& image)` | 获取一帧图像 |
| `is_opened()` | 检查相机是否处于打开状态 |
| `last_error()` | 获取最近一次错误信息 |
| `stop_grabbing()` | 停止图像采集 |
| `close()` | 关闭相机设备 |
| `finalize()` | 释放 MVS SDK 资源 |

`start()` 内部会依次完成初始化、设备枚举、设备打开、相机参数配置和图像采集启动。

正常使用时，只需要创建对象、调用 `start()`，再通过 `get_frame()` 获取图像。

## 在其他项目中使用

本库支持两种接入方式。

### 方式一：通过 add_subdirectory() 接入源码

适用于 Git Submodule 或者需要与 Camera 源码一起开发的项目。

推荐目录结构：

```text
MyProject/
├── CMakeLists.txt
├── app/
│   └── main.cpp
└── Camera/
    └── ...
```

在主项目的 `CMakeLists.txt` 中添加：

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyProject LANGUAGES CXX)

add_subdirectory(Camera)

add_executable(my_app
    app/main.cpp
)

target_link_libraries(my_app
    PRIVATE
        Camera::Camera
)
```

在源文件中直接包含：

```cpp
#include <Camera.h>
```

### 方式二：通过 find_package() 接入安装后的库

适用于已经安装 Camera，希望多个独立项目共享同一份库的场景。

首先在 Camera 项目中编译：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

cmake --build build --parallel
```

安装到用户目录：

```bash
cmake --install build --prefix "$HOME/.local"
```

安装完成后，在其他项目的 `CMakeLists.txt` 中添加：

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyProject LANGUAGES CXX)

find_package(Camera CONFIG REQUIRED)

add_executable(my_app
    app/main.cpp
)

target_link_libraries(my_app
    PRIVATE
        Camera::Camera
)
```

配置消费者项目时，指定 Camera 的安装前缀：

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$HOME/.local"
```

然后编译：

```bash
cmake --build build --parallel
```

## 独立测试

项目提供了独立测试程序 `tests/camera_test.cpp`，用于验证相机启动、图像采集和资源生命周期。

开启测试选项：

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_CAMERA_TESTS=ON
```

编译：

```bash
cmake --build build --parallel
```

运行：

```bash
./build/camera_test
```

测试程序需要连接真实相机，并且系统已经正确安装 MVS SDK 和 Runtime。

## 项目结构

```text
Camera/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── cmake/
│   └── CameraConfig.cmake.in
├── include/
│   └── Camera.h
├── src/
│   └── Camera.cpp
└── tests/
    └── camera_test.cpp
```

| 目录或文件 | 作用 |
|---|---|
| `include/` | 对外公开的头文件 |
| `src/` | Camera 内部实现 |
| `tests/` | 独立测试程序 |
| `cmake/` | CMake 安装包配置模板 |
| `CMakeLists.txt` | 项目构建和安装配置 |
| `README.md` | 项目说明和使用文档 |

## 注意事项

- 本项目目前面向 Linux 环境。
- 使用前需要安装 OpenCV 和海康机器人 MVS SDK。
- `get_frame()` 获取的图像为 BGR 格式的 `cv::Mat`。
- `Camera` 对象禁止拷贝和移动，避免相机资源被多个对象错误管理。
- 正常情况下，`Camera` 对象离开作用域时会自动释放资源。
- 测试程序需要实际连接相机，不能仅通过编译成功判断设备采集是否正常。

## License

License information will be added before public release.