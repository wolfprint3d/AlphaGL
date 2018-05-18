#pragma once
#include "SceneNode.h"


namespace AGL
{

    class DLLEXPORT Camera : public SceneNode
    {
    public:
        static constexpr SceneNodeType NodeType = SN_Camera;

        Matrix4 View, Projection, ViewProjection;

        float Fov = 45.0f;
        Vector3 Target = Vector3::ZERO;
        bool AutoLookAtTarget = false;

        Camera(GLCore& core, SceneNode* parent, string name, int typeFlags = 0);
        ~Camera();

        void LookAtTarget(Vector3 target);
        void ClearLookAt();
        void UpdateViewProjection();
    };

}

