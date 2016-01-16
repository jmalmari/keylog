#include <iostream>
#include <signal.h>
#include <string.h>
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
    char const* device = nullptr;
    char const* name = nullptr;
    char const* dbpath = nullptr;

    for (int i = 1; i < argc; ++i)
    {
        char const* const argName = argv[i];

        if (argName[0] == '-' && (i + 1) < argc)
        {
            ++i;

            char const* const argValue = argv[i];

            switch (argName[1])
            {
            case 'd':
                device = argValue;
                break;
            case 'n':
                name = argValue;
                break;
            case 'f':
                dbpath = argValue;
                break;
            default:
                std::cerr << "unknown arg " << argName[1] << '\n';
                break;
            }
        }
    }

    if (!device || !name)
    {
        std::cerr << "switches:\n"
            "\t -d <event device path>\n"
            "\t -n <descriptive device name>\n"
            "\t[-f <database file>]\n";
        return 1;
    }

    if (!dbpath)
    {
        dbpath = "keylog.db";
    }

    std::cout << "Configuration:\n"
              << "  input device\t" << name << " (" << device << ")\n"
              << "  database\t" << dbpath << "\n"
              << std::flush;

    (void)signal(SIGINT, &handleSignal);

    InputDevice input(device);

    KeyLog keylog(dbpath, name);
    input.registerListener(&keylog);

    while (input.read() && !g_quit)
    {
        keylog.printStats(std::cout);
    }

    input.unregisterListener(&keylog);

    std::cout << "-- \nkthanksbye\n";

    return 0;
}
