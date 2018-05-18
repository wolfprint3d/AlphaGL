#pragma once
#include "Shader.h"
#include <memory>
#include "GLCore.h"

namespace AGL
{
    using std::unique_ptr;
    /////////////////////////////////////////////////////////////////////////////////

    enum SceneNodeType : int
    {
        SN_None     = 0,
        SN_Actor    = (1<<0),
        SN_RootNode = (1<<1),
        SN_Camera   = (1<<2),
    };

    class SceneNode;
    using SceneNodeRef = unique_ptr<SceneNode>;

    /**
     * @brief Base class for scene nodes, may not be renderable
     */
    class SceneNode
    {
    public:
        static constexpr SceneNodeType NodeType = SN_None;

    private:
        SceneNodeType Type;

    protected:
        GLCore& Core;
        SceneNode* Parent;
        string NodeName;
        vector<SceneNodeRef> ChildNodes;

        friend class Camera;

    public:

        Vector3 Position = Vector3::ZERO;
        Vector3 Scale    = Vector3::ONE;
        Vector3 Rotation = Vector3::ZERO;

        SceneNode(GLCore& core, SceneNode* parent, string name, int typeFlags = 0);
        virtual ~SceneNode();

        // we want pointers to SceneNode to remain stable, so any copy/move forbidden
        SceneNode& operator=(const SceneNode&) = delete;
        SceneNode& operator=(SceneNode&&) = delete;
        SceneNode(const SceneNode&) = delete;
        SceneNode(SceneNode&&) = delete;

        strview Name() const { return NodeName; }

        const vector<SceneNodeRef>& Nodes() const { return ChildNodes; }

        // fast type check
        inline bool Is(SceneNodeType type) const { return (Type & type) != 0; }
        
        // fast casting
        template<class TNode> TNode* Cast()
        {
            return Is(TNode::NodeType) ? static_cast<TNode*>(this) : nullptr;
        }
        template<class TNode> const TNode* Cast() const
        {
            return Is(TNode::NodeType) ? static_cast<const TNode*>(this) : nullptr;
        }
        
        // creates a new child node
        template<class TNode> TNode* CreateNode(string name)
        {
            TNode* node = new TNode(Core, this, move(name), TNode::NodeType);
            ChildNodes.emplace_back(SceneNodeRef{ node });
            return node;
        }
        
        class Actor* CreateActor(string name, AGL::VertexBuffer&& mesh);
        class Actor* CreateActor(string name, AGL::VertexBuffer&& mesh, Color color);
        class Actor* CreateActor(string name, AGL::VertexBuffer&& mesh, const AGL::Texture& texture);
        class Actor* CreateActor(string name, AGL::VertexBuffer&& mesh, const AGL::Texture* texture);

        class Actor* FindActor(const string& name) const;

        template<class TActor> TActor* FindActor(const string& name) const
        {
            if (NodeName == name)
                return (TActor*)Cast<TActor>();
            for (const SceneNodeRef& node : ChildNodes)
                if (auto* actor = node->FindActor(name))
                    return actor;
            return nullptr;
        }

        Matrix4 LocalTransform() const;
        Matrix4 WorldTransform(const Matrix4& parentWorld) const;
        Matrix4 GetParentWorldTransform() const;

        // called every frame, updates self and all child nodes
        virtual void Update(float deltaTime);
        virtual void Render(const Matrix4& parentWorld, const Matrix4& viewProjection);
    };

    /////////////////////////////////////////////////////////////////////////////////
}

