#pragma once
#include "SceneNode.h"
#include "Actor.h" // purely for convenience
#include "Camera.h"

namespace AGL
{
    /////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Root node of the scenegraph
     */
    class SceneRoot : public SceneNode
    {
    public:
        static constexpr SceneNodeType NodeType = SN_RootNode;

        Color3 BackgroundColor = Color3::White();

        /**
         * The default camera is always created in the scene
         */
        Camera* Camera;

        /**
         * Parent camera arm for easily rotating the camera around objects
         */
        SceneNode* CameraArm;

        explicit SceneRoot(GLCore& core, string sceneName);

        void Update(float deltaTime) override;
        void Render();

    private:
        void Render(const Matrix4& parentWorld, const Matrix4& viewProjection) override;
    };

    /////////////////////////////////////////////////////////////////////////////////
}

