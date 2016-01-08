#ifndef IINPUTLISTENER_HPP
#define IINPUTLISTENER_HPP

struct KeyEvent;

class IInputListener
{
public:
    virtual ~IInputListener() {}

protected:
    virtual void onKeyEvent(KeyEvent const& event) = 0;
    friend class InputDevice;
};

#endif
