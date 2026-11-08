#include "Camera.h"

#include <iostream>

#include <opencv2/core.hpp>

// 每次调用都创建一个独立的 Camera 对象。
bool test_camera_once(int run_id)
{
    std::cout << "\n========== Camera test "
              << run_id << " ==========" << std::endl;

    // Camera 的生命周期从这里开始。
    Camera camera;

    if (!camera.start())
    {
        std::cerr << "Failed to start camera: "
                  << camera.last_error() << std::endl;

        return false;
    }

    std::cout << "Camera started successfully." << std::endl;

    cv::Mat frame;

    if (!camera.get_frame(frame))
    {
        std::cerr << "Failed to get frame: "
                  << camera.last_error() << std::endl;

        return false;
    }

    std::cout << "Frame received successfully." << std::endl;
    std::cout << "Image size: "
              << frame.cols << " x " << frame.rows << std::endl;
    std::cout << "Image channels: "
              << frame.channels() << std::endl;

    // 函数返回时，camera 自动析构。
    // Camera::~Camera() 会销毁 Impl，
    // Impl::~Impl() 会调用 finalize()。
    return true;
}

int main()
{
    // 第一轮：创建、启动、采集、销毁。
    if (!test_camera_once(1))
    {
        return 1;
    }

    std::cout << "First Camera object destroyed." << std::endl;

    // 第二轮：验证第一次销毁后能否重新初始化。
    if (!test_camera_once(2))
    {
        return 1;
    }

    std::cout << "Second Camera object destroyed." << std::endl;

    std::cout << "\nRAII lifecycle test completed." << std::endl;

    return 0;
}
