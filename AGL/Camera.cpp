#include "Camera.h"

namespace AGL
{
    Camera::Camera(GLCore& core, SceneNode* parent, string name, int typeFlags)
      : SceneNode{core, parent, std::move(name), typeFlags|NodeType},
        View           { Matrix4::Identity() },
        Projection     { Matrix4::Identity() },
        ViewProjection { Matrix4::Identity() }
    {
    }
    Camera::~Camera()
    {
    }
    void Camera::LookAtTarget(Vector3 target)
    {
        Target = target;
        AutoLookAtTarget = true;
    }
    void Camera::ClearLookAt()
    {
        Target = Vector3::Zero();
        AutoLookAtTarget = false;
    }

    void Camera::UpdateViewProjection()
    {
        if (AutoLookAtTarget)
        {
            View = Matrix4::createLookAt(Position, Target, Vector3::Up());
        }
        else
        {
            View = Matrix4::createAffine3D(Position, Scale, Rotation);
        }

        Projection = Matrix4::createPerspective(
            Fov, Core.ContextWidth(), Core.ContextHeight(), 0.001f, 10000.0f);
        ViewProjection = Projection * View * GetParentWorldTransform();

    }
}
