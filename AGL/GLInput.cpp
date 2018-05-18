#include "GLInput.h"
#if __linux__
    #include "GLContext_Linux.h"
#elif _WIN32
    #include "GLContext_Win32.h"
#endif
#include <rpp/collections.h>

namespace AGL
{
    using std::move;
    using rpp::nearlyZero;
    
    GLInput::GLInput()  = default;
    GLInput::~GLInput() = default;

    void GLInput::Init(void* windowContext)
    {
        Context = windowContext;
#if USING_GLFW
        GLFWwindow* window = (GLFWwindow*)windowContext;
        glfwSetWindowUserPointer(window, this);
        glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered)
        {
            GLInput& in = *(GLInput*)glfwGetWindowUserPointer(window);
            in.InFocus = entered != 0;
        });
        glfwSetScrollCallback(window, [](GLFWwindow* window, double dx, double dy)
        {
            GLInput& in = *(GLInput*)glfwGetWindowUserPointer(window);
            in.MouseDelta.z = (float)dy;
            in.MousePos.z += (float)dy;
            for (const InputHandler<MouseMoved>& mouseMoved : in.MouseMoves)
                mouseMoved.handler(in.MousePos, in.MouseDelta);
        });
        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y)
        {
            GLInput& in = *(GLInput*)glfwGetWindowUserPointer(window);
            if (in.MousePos.x != 0.0f || in.MousePos.y != 0.0f)
            {
                in.MouseDelta.x = float(x) - in.MousePos.x;
                in.MouseDelta.y = float(y) - in.MousePos.y;
            }
            in.MousePos.x = float(x);
            in.MousePos.y = float(y);
            for (const InputHandler<MouseMoved>& mouseMoved : in.MouseMoves)
                mouseMoved.handler(in.MousePos, in.MouseDelta);
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
        {
            GLInput& in = *(GLInput*)glfwGetWindowUserPointer(window);
            for (const InputHandler<MouseClicked>& mouseClicked : in.MouseClicks)
                mouseClicked.handler(MouseButton(button), action == GLFW_PRESS);
        });
        glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            GLInput& in = *(GLInput*)glfwGetWindowUserPointer(window);
            for (const InputHandler<KeyEvent>& keyPressed : in.KeyEvents)
                keyPressed.handler(key, KeyModifiers(mods), KeyAction(action));
        });
        glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codepoint)
        {
            GLInput& in = *(GLInput*)glfwGetWindowUserPointer(window);
            for (const InputHandler<TextInput>& textInput : in.TextEvents)
                textInput.handler(codepoint);
        });
#endif
    }

    void GLInput::PollEvents()
    {
        MouseDelta = Vector3::ZERO;
    #if USING_GLFW
        glfwPollEvents();
    #endif
    }

    bool GLInput::IsKeyDown(int key) const
    {
    #if USING_GLFW
        int s = glfwGetKey((GLFWwindow*)Context, key);
        return s == GLFW_PRESS || s == GLFW_REPEAT;
    #else
        return false;
    #endif
    }

    bool GLInput::IsMouseDown(MouseButton button) const
    {
    #if USING_GLFW
        int s = glfwGetMouseButton((GLFWwindow*)Context, button);
        return s == GLFW_PRESS || s == GLFW_REPEAT;
    #else
        return false;
    #endif
    }

    void GLInput::AddKeyListener(void* id, KeyEvent&& listener)
    {
        KeyEvents.emplace_back(id, move(listener));
    }
    void GLInput::AddTextListener(void* id, TextInput&& listener)
    {
        TextEvents.emplace_back(id, move(listener));
    }
    void GLInput::AddClickListener(void* id, MouseClicked&& listener)
    {
        MouseClicks.emplace_back(id, move(listener));
    }
    void GLInput::AddMoveListener(void* id, MouseMoved&& listener)
    {
        MouseMoves.emplace_back(id, move(listener));
    }

    template<class T> void EraseListener(vector<InputHandler<T>>& events, void* id)
    {
        rpp::erase_first_if(events, [id](const InputHandler<T>& evt) {
            return evt.id == id;
        });
    }

    void GLInput::RemoveKeyListener(void* id)
    {
        EraseListener(KeyEvents, id);
    }
    void GLInput::RemoveTextListener(void* id)
    {
        EraseListener(TextEvents, id);
    }
    void GLInput::RemoveClickListener(void* id)
    {
        EraseListener(MouseClicks, id);
    }
    void GLInput::RemoveMoveListener(void* id)
    {
        EraseListener(MouseMoves, id);
    }
}
