#include <iostream>
#include <signal.h>
#include "input/inputdevice.hpp"
#include "log/keylog.hpp"

namespace {

bool g_quit = false;

}

void handleSignal(int signal)
{
    switch (signal)
    {
    case SIGINT:
        g_quit = true;
        break;
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    char const* const defaultDeviceName = "/dev/input/ts";
    char const* deviceName = defaultDeviceName;

    if (1 < argc && argv[1] && *argv[1])
    {
        deviceName = argv[1];
    }

    std::cout << "using input device " << deviceName << std::endl;

    (void)signal(SIGINT, &handleSignal);

    InputDevice input(deviceName);

    KeyLog keylog("keylog.db");
    input.registerListener(&keylog);

    while (input.read() && !g_quit)
    {
        keylog.printStats(std::cout);
    }

    input.unregisterListener(&keylog);

    std::cout << "-- \nkthanksbye\n";

    return 0;
}
