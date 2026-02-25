#include <iostream>
#include <libcamera/libcamera.h>
#include <memory>

using namespace libcamera;

int main()
{
    CameraManager cm;
    cm.start();

    if (cm.cameras().empty()) {
        std::cout << "No Camera Found!" << std::endl;
        cm.stop();
        return -1;
    }

    std::shared_ptr<Camera> camera = cm.cameras()[0];
    camera->acquire();

    std::cout << "Camera is available" << std::endl;

    camera->release();
    cm.stop();
    return 0;
}