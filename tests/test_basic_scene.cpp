#include <AGL/SceneRoot.h>
#include <rpp/tests.h>

TestImpl(test_basic_scene)
{
    std::unique_ptr<AGL::GLCore> Core;
    AGL::SceneRoot* Root      = nullptr;
    AGL::Camera*    Camera    = nullptr;

    TestInit(test_basic_scene)
    {
        Core = std::make_unique<AGL::GLCore>(800, 600, /*createWindow:*/true);
        Root = Core->CreateSceneRoot("basic_scene");
        Root->BackgroundColor = rpp::Color3::RGB(125, 125, 125);
        Core->SetTitle("test_basic_scene");

        Camera = Root->Camera;
        Camera->Position = { 0.f, 10.f, +35.f };
        Camera->LookAtTarget({ 0.f, 15.f, 0.f });
    }

    TestCase(render_frame)
    {
        Core->UpdateAndRender();
    }

};
