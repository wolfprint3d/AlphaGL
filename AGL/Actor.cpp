#include "Actor.h"
#include <rpp/debugging.h>
#include "OpenGL.h"

namespace AGL
{
    /////////////////////////////////////////////////////////////////////////////////

    Actor::Actor(GLCore& core, SceneNode* parent, string name, int typeFlags)
        : SceneNode(core, parent, move(name), typeFlags|NodeType)
    {
    }

    void Actor::Update(float deltaTime)
    {
        SceneNode::Update(deltaTime);
    }

#define CheckGLResult(expression, what) do { \
        expression; \
        if (auto err = AGL::glGetErrorStr()) LogError("%s failed: %s", what, err); \
    } while (false)

    void Actor::Render(const Matrix4& parentWorld, const Matrix4& viewProjection)
    {
        Matrix4 worldTransform = WorldTransform(parentWorld);
        if (Mesh)
        {
            Matrix4 modelViewProj = viewProjection * worldTransform;
            if (!Mat.shader)
            {
                if      (Mat.texture)        Mat.shader = &Core.Simple3dShader();
                else if (Mesh.hasAttrib(a_Color)) Mat.shader = &Core.VertexColor3dShader();
                else                              Mat.shader = &Core.Color3dShader();
            }

            Shader& shader = *Mat.shader;
            CheckGLResult(shader.bind(), "shader.bind()");
            CheckGLResult(shader.bind(u_Transform, modelViewProj), "shader.bind(u_Transform)");
            CheckGLResult(shader.bind(u_DiffuseColor, Mat.color), "shader.bind(u_DiffuseColor)");
            if (Mat.texture)
                CheckGLResult(shader.bind(u_DiffuseTex, Mat.texture.texture), "shader.bind(u_DiffuseTex)");
            CheckGLResult(Mesh.draw(), "mesh.draw()");
        }

        for (int i = 0; i < (int)ChildNodes.size(); ++i)
        {
            SceneNode* node = ChildNodes[i].get();
            node->Render(worldTransform, viewProjection);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////
}

