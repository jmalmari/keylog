#ifndef INPUTDEVICE_HPP
#define INPUTDEVICE_HPP

#include <fstream>
#include <string>
#include <list>
#include <linux/input.h>

class IInputListener;

class InputDevice
{
public:
    InputDevice(std::string const& devicePath);
    virtual ~InputDevice();

    bool read();

    void registerListener(IInputListener* listener);
    void unregisterListener(IInputListener* listener);

private:
    struct EventContext;

    void notifyEvents(EventContext const& ctx);
    bool readKernelEvent(input_event& event);
    bool handleKernelEvent(EventContext& ctx, input_event const& event);

private:
    std::string const _devicePath;
    std::ifstream _device;
    std::list<IInputListener*> _listeners;
};

#endif
