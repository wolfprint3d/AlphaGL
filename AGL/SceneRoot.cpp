#include "SceneRoot.h"

namespace AGL
{
    /////////////////////////////////////////////////////////////////////////////////


    SceneRoot::SceneRoot(GLCore& core, string sceneName)
        : SceneNode(core, nullptr, move(sceneName), NodeType)
    {
        CameraArm = CreateNode<SceneNode>("cameraArm");
        Camera = CameraArm->CreateNode<AGL::Camera>("camera");

        // default camera pos is looking into the screen from Z=25.0
        // Y is up, Z is out of the screen
        Camera->Position = { 0.0f, 0.0f, 25.0f };
        Camera->Rotation = { 0.0f, 0.0f, 0.0f  };
    }

    void SceneRoot::Update(float deltaTime)
    {
        SceneNode::Update(deltaTime);
    }

    void SceneRoot::Render(const Matrix4& parentWorld, const Matrix4& viewProjection)
    {
        Core.Clear(BackgroundColor);
        for (SceneNodeRef& node : ChildNodes)
            node->Render(parentWorld, viewProjection);
        Core.SwapBuffers();
    }

    void SceneRoot::Render()
    {
        Camera->UpdateViewProjection();
        Render(Matrix4::Identity(), Camera->ViewProjection);
    }

    /////////////////////////////////////////////////////////////////////////////////
}
