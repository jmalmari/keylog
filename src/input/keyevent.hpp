#ifndef KEYEVENT_HPP
#define KEYEVENT_HPP

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

#endif
