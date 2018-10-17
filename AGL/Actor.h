#pragma once
#include "SceneNode.h"

namespace AGL
{
    /////////////////////////////////////////////////////////////////////////////////

    /** 
     * @brief Basic VISIBLE scene object
     */
    class AGL_API Actor : public SceneNode
    {
    public:
        static constexpr SceneNodeType NodeType = SN_Actor;

        VertexBuffer Mesh;
        Material Material;

        Actor(GLCore& core, SceneNode* parent, string name, int typeFlags = 0);

        void Update(float deltaTime) override;
        void Render(const Matrix4& parentWorld, const Matrix4& viewProjection) override;
    };

    /////////////////////////////////////////////////////////////////////////////////
}
