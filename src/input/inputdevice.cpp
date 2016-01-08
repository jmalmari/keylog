#include <iostream>
#include "inputdevice.hpp"
#include "iinputlistener.hpp"
#include "keyevent.hpp"

#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace {

char const* const eventTypeName(int type)
{
    switch (type)
    {
    case EV_SYN:
        return "EV_SYN";
    case EV_KEY:
        return "EV_KEY";
    case EV_REL:
        return "EV_REL";
    case EV_ABS:
        return "EV_ABS";
    case EV_MSC:
        return "EV_MSC";
    case EV_SW:
        return "EV_SW";
    case EV_LED:
        return "EV_LED";
    case EV_SND:
        return "EV_SND";
    case EV_REP:
        return "EV_REP";
    case EV_FF:
        return "EV_FF";
    case EV_PWR:
        return "EV_PWR";
    case EV_FF_STATUS:
        return "EV_FF_STATUS";
    default:
        return "unknown";
    }
}

struct MtContext
{
    int trackingId = -1;
    int absX = 0;
    int absY = 0;
};

} // end unnamed namespace

struct InputDevice::EventContext
{
    bool complete = false;
    std::string indent;
    MtContext mt;
    KeyEvent key;
};

InputDevice::InputDevice(std::string const& devicePath) :
    _devicePath(devicePath),
    _device(devicePath)
{
}

InputDevice::~InputDevice()
{
}

bool InputDevice::read()
{
    EventContext ctx;
    ctx.complete = false;

    while (!ctx.complete)
    {
        input_event event;

        if (!readKernelEvent(event))
        {
            return false;
        }

        if (!handleKernelEvent(ctx, event))
        {
            std::cout << std::hex
                      << "read unhandled event " << eventTypeName(event.type) << "\n"
                      << "  type: 0x" << event.type << '\n'
                      << "  code: 0x" << event.code << '\n'
                      << " value: 0x" << event.value
                      << std::endl;
        }
    }

    notifyEvents(ctx);

    return true;
}

void InputDevice::registerListener(IInputListener* listener)
{
    if (listener)
    {
        _listeners.push_back(listener);
    }
}

void InputDevice::unregisterListener(IInputListener* listener)
{
    if (listener)
    {
        _listeners.remove(listener);
    }
}

void InputDevice::notifyEvents(EventContext const& ctx)
{
    for (IInputListener* listener : _listeners)
    {
        if (0 <= ctx.key.key)
        {
            listener->onKeyEvent(ctx.key);
        }
    }
}

bool InputDevice::handleKernelEvent(EventContext& ctx, input_event const& event)
{
    bool handled = false;

    switch (event.type) {
    case EV_SYN:
        /* - Used as markers to separate events. Events may be separated in time or in
           space, such as with the multitouch protocol. */

        if (event.code == SYN_MT_REPORT) {
            // sync single touch point
            if (0 <= ctx.mt.trackingId) {
                std::cout << ctx.indent << "SYN_MT_REPORT: Tracking id "
                          << ctx.mt.trackingId
                          << ": ("
                          << ctx.mt.absX << ","
                          << ctx.mt.absY << ")" << std::endl;
                ctx.mt.trackingId = -1;
            } else {
                std::cout << ctx.indent << "SYN_MT_REPORT: empty" << std::endl;
            }
        } else if (event.code == SYN_REPORT) {
            ctx.complete = true;
        } else {
            std::cout << ctx.indent << std::hex
                      << "sync marker with code 0x" << event.code
                      << ", value: 0x" << event.value << std::endl;
        }
        handled = true;
        break;
    case EV_KEY:
        if (event.code == BTN_TOUCH) {
            if (event.value == 1) { // Start
                std::cout  << "\nBegin touch" << std::endl;
                ctx.indent += '\t';
            } else if (event.value == 0) { // End
                ctx.indent.erase(ctx.indent.end() - 1);
                std::cout << "End touch" << std::endl;
            } else {
                std::cout << std::hex
                          << "EV_KEY: BTN_TOUCH, value: 0x" << event.value
                          << std::endl;
            }
            handled = true;
        } else if (event.code < 0x100) {
            ctx.key.key = event.code;

            switch (event.value)
            {
            case 0:
                ctx.key.action = KeyEvent::KeyReleased;
                break;
            case 1:
                ctx.key.action = KeyEvent::KeyPressed;
                break;
            case 2:
                ctx.key.action = KeyEvent::KeyRepeated;
                break;
            }

            handled = true;
        }
        break;
    case EV_ABS:
        /* - Used to describe absolute axis value changes, e.g. describing the
           coordinates of a touch on a touchscreen. */

        switch (event.code) {
        case ABS_X:
            break;
        case ABS_Y:
            break;
        case ABS_MT_TRACKING_ID:
            ctx.mt.trackingId = event.value;
            handled = true;
            break;
        case ABS_MT_TOUCH_MAJOR:
            break;
        case ABS_MT_POSITION_X:
            ctx.mt.absX = event.value;
            handled = true;
            break;
        case ABS_MT_POSITION_Y:
            ctx.mt.absY = event.value;
            handled = true;
            break;
        case FF_FRICTION:
            break;
        default:
            break;
        }
        break;

    case EV_MSC:
        if (event.code == MSC_SCAN)
        {
            ctx.key.scancode = event.value;
            handled = true;
        }

        break;

    default:
        break;
    }

    return handled;
}

bool InputDevice::readKernelEvent(input_event& event)
{
    return _device.read(reinterpret_cast<char*>(&event),
                        sizeof(input_event));
}
