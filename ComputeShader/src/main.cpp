
#include <iostream>

#include "instance_.h"

namespace cs
{
void initApplication()
{
    DEBUG_MESSAGE_NORMAL("initApplication")

    Instance::init();
}

void quitApplication()
{
    DEBUG_MESSAGE_NORMAL("quitApplication");

    Instance::quit();
}
}

int main(int argv, char** argc)
{
    std::cout << "compute shader" << std::endl;

    cs::initApplication();

    cs::quitApplication();
    return 0;
}
