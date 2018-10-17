#include "SceneNode.h"
#include "Actor.h"

namespace AGL
{
    /////////////////////////////////////////////////////////////////////////////////

    SceneNode::SceneNode(GLCore& core, SceneNode* parent, string name, int typeFlags)
        : Type(SceneNodeType(typeFlags)), Core(core), Parent(parent), NodeName(std::move(name))
    {
    }

    SceneNode::~SceneNode()
    {
        // manually clear nodes to catch destruction time errors inside debugger
        #if _DEBUG || _WIN32
            ChildNodes.clear();
        #endif
    }

    Actor* SceneNode::CreateActor(string name)
    {
        return CreateNode<Actor>(name);
    }

    Actor* SceneNode::CreateActor(string name, AGL::VertexBuffer&& mesh)
    {
        auto actor = CreateNode<Actor>(name);
        actor->Mesh = std::move(mesh);
        return actor;
    }

    Actor* SceneNode::CreateActor(string name, AGL::VertexBuffer&& mesh, Color color)
    {
        auto actor = CreateNode<Actor>(name);
        actor->Mesh = std::move(mesh);
        actor->Mat.color = color;
        return actor;
    }

    Actor* SceneNode::CreateActor(string name, AGL::VertexBuffer&& mesh, const AGL::Texture& texture)
    {
        auto actor = CreateNode<Actor>(name);
        actor->Mesh = std::move(mesh);
        actor->Mat.texture = texture;
        return actor;
    }

    Actor* SceneNode::CreateActor(string name, AGL::VertexBuffer&& mesh, const AGL::Texture* texture)
    {
        auto actor = CreateNode<Actor>(name);
        actor->Mesh = std::move(mesh);
        actor->Mat.texture = texture;
        return actor;
    }

    Actor* SceneNode::FindActor(const string& name) const
    {
        return FindActor<Actor>(name);
    }

    Matrix4 SceneNode::LocalTransform() const
    {
        return Matrix4::createAffine3D(Position, Scale, Rotation);
    }

    Matrix4 SceneNode::WorldTransform(const Matrix4& parentWorld) const
    {
        return parentWorld * LocalTransform();
    }

    Matrix4 SceneNode::GetParentWorldTransform() const
    {
        Matrix4 world = Matrix4::Identity();
        for (auto* parent = Parent; parent; parent = parent->Parent)
            world = parent->LocalTransform() * world;
        return world;
    }

    void SceneNode::Update(float deltaTime)
    {
        for (int i = 0; i < (int)ChildNodes.size(); ++i) // @note Loop must tolerate removals
        {
            SceneNode* node = ChildNodes[i].get();
            node->Update(deltaTime);
        }
    }

    void SceneNode::Render(const Matrix4& parentWorld, const Matrix4& viewProjection)
    {
        Matrix4 worldTransform = WorldTransform(parentWorld);
        for (int i = 0; i < (int)ChildNodes.size(); ++i) // @note Loop must tolerate removals
        {
            SceneNode* node = ChildNodes[i].get();
            node->Render(worldTransform, viewProjection);
        }
    }


    /////////////////////////////////////////////////////////////////////////////////
}
