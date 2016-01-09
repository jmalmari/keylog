#ifndef KEYEVENT_HPP
#define KEYEVENT_HPP

#include <ostream>

struct KeyEvent
{
    enum KeyAction
    {
        KeyUnknownAction,
        KeyPressed,
        KeyReleased,
        KeyRepeated,
    };

    int key = -1;
    int scancode = -1;
    KeyAction action = KeyUnknownAction;
};

inline std::ostream& operator<<(std::ostream& os, KeyEvent::KeyAction action)
{
    switch (action)
    {
    case KeyEvent::KeyUnknownAction:
        return os << "KeyUnknownAction";
    case KeyEvent::KeyPressed:
        return os << "KeyPressed";
    case KeyEvent::KeyReleased:
        return os << "KeyReleased";
    case KeyEvent::KeyRepeated:
        return os << "KeyRepeated";
    }
}

#endif
