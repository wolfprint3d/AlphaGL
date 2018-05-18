#pragma once
#include "AGLConfig.h"
#include <rpp/delegate.h>
#include <vector>
#include <rpp/vec.h>

namespace AGL
{
    using rpp::delegate;
    using std::vector;
    using std::pair;
    using rpp::Vector3;

    enum MouseButton
    {
        MB_Left,
        MB_Right,
        MB_Middle,
    };

    enum KeyAction
    {
        KeyRelease,
        KeyPress,
        KeyRepeat
    };

    enum KeyModifiers
    {
        ModifierNone  = 0,
        ModifierShift = (1 << 0),
        ModifierCtrl  = (1 << 1),
        ModifierAlt   = (1 << 2),
        ModifierSuper = (1 << 3), // Windows | Mac key
    };

    using KeyEvent     = delegate<void(int key, KeyModifiers mod, KeyAction action)>;
    using TextInput    = delegate<void(unsigned int codepoint)>;
    using MouseClicked = delegate<void(MouseButton button, bool pressed)>;
    using MouseMoved   = delegate<void(Vector3 pos, Vector3 delta)>;

    template<class T> struct InputHandler
    {
        void* id;
        T handler;
        InputHandler(void* id, T&& handler) : id{id}, handler{std::move(handler)}
        {
        }
    };

    class DLLEXPORT GLInput
    {
        void* Context = nullptr;
        vector<InputHandler<KeyEvent>>     KeyEvents;
        vector<InputHandler<TextInput>>    TextEvents;
        vector<InputHandler<MouseClicked>> MouseClicks;
        vector<InputHandler<MouseMoved>>   MouseMoves;

        Vector3 MousePos   = Vector3::ZERO;
        Vector3 MouseDelta = Vector3::ZERO;
        bool InFocus = false;

    public:

        GLInput();
        ~GLInput();

        void Init(void* windowContext);
        void PollEvents();

        bool IsKeyDown(int key) const;
        bool IsMouseDown(MouseButton button) const;

        void AddKeyListener(void* id, KeyEvent&& listener);
        void AddTextListener(void* id, TextInput&& listener);
        void AddClickListener(void* id, MouseClicked&& listener);
        void AddMoveListener(void* id, MouseMoved&& listener);

        void RemoveKeyListener(void* id);
        void RemoveTextListener(void* id);
        void RemoveClickListener(void* id);
        void RemoveMoveListener(void* id);
    };
}
