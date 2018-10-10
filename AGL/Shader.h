//
// Created by Jorma on 24/05/16.
//
#pragma once
#include "Texture.h"
#include <cstdint>

namespace AGL
{
    using std::swap;
    using std::vector;
    using rpp::Color;
    using rpp::Vector3;
    using rpp::Vector4;
    using rpp::Matrix4;
    ////////////////////////////////////////////////////////////////////////////////


    typedef unsigned int index_t; // vertex index type 

    /** @brief shader uniform slots */
    enum ShaderUniform : int
    {
        u_Transform,    // uniform matrix transform;      model-view-project matrix
        u_DiffuseTex,   // uniform sampler2D diffuseTex;  diffuse texture
        u_SpecularTex,  // uniform sampler2D specularTex; specular texture
        u_NormalTex,    // uniform sampler2D normalTex;   normal texture
        u_ShadowTex,    // uniform sampler2D shadowTex;   shadow texture
        u_OccludeTex,   // uniform sampler2D occludeTex;  occlusion texture for fake SSAO
        u_DiffuseColor, // uniform vec4 diffuseColor;     diffuse color 
        u_OutlineColor, // uniform vec4 outlineColor;     background or outline color
        u_ShaderData,   // uniform vec4 shaderData;       shader specific data
        u_MaxUniforms,  // uniform counter
    };


    /** @brief shader attribute slots */
    enum ShaderAttr : int
    {
        a_Position,      // attribute vec3 position;    position (vec2 XY or vec3 XYZ)
        a_Normal,        // attribute vec3 normal;      normal 
        a_Coord,         // attribute vec2 coord;       texture coordinate 0
        a_Coord2,        // attribute vec2 coord2;      texture coordinate 1
        a_Vertex,        // attribute vec4 vertex;      additional generic 4D vertex
        a_Color,         // attribute vec4 color;       per-vertex coloring
        a_MaxAttributes, // attribute counter
    };


    ////////////////////////////////////////////////////////////////////////////////


    /** @brief Describes a single element in a vertex (visualized by .natvis) */
    struct VertexDescrElem
    {
        uint8_t attr; // ShaderAttr vertex attribute slot identifier (a_Position, etc.)
        uint8_t size; // Number of elements per attribute (1-4 floats or ints)
    };


    /** @brief vertex layout descriptor (visualized by .natvis) */
    struct VertexDescr
    {
        int sizeOf; // Size of your vertex struct in bytes eg: sizeof(Vertex3UV)
        VertexDescrElem items[4];

        // creates an empty vertex descriptor
        VertexDescr() noexcept;

        // constructs a new vertex descriptor performs layout validation
        VertexDescr(int sizeOf, ShaderAttr attr0, int size0, ...) noexcept;

        // validate this vertex layout descriptor
        void validate() const;
        
        // resets layout descriptor
        void clear();

        bool operator==(const VertexDescr& d) const { 
            return sizeOf == d.sizeOf && memcmp(items, d.items, sizeof(items)) == 0;
        }
        bool operator!=(const VertexDescr& d) const { 
            return sizeOf != d.sizeOf || memcmp(items, d.items, sizeof(items)) != 0;
        }
    };

    #undef countof
    
    // simple utility for getting the number of elements in a plain C array
    template<class T, int N> constexpr int countof(const T (&arr)[N]) {
        return N;
    }

    ////////////////////////////////////////////////////////////////////////////////

    /**
     * @note Describes a simple 2D vertex with only position attribute
     * @note Attribute a_Position
     */
    struct Vertex2 // basic 2D vertex
    {
        float x, y; // position xy
        static VertexDescr layout() {
            return{ sizeof(Vertex2), a_Position,2 };
        }
    };

    /**
     * @note Describes a 2D vertex with UV coordinates
     * @note Attributes a_Position, a_Coord
     */
    struct Vertex2UV // basic 2D vertex with UV coordinates
    {
        float x, y; // position xy
        float u, v; // texture coordinates
        static VertexDescr layout() {
            return { sizeof(Vertex2UV), a_Position,2, a_Coord,2 };
        }
    };
    

    /**
     * @note Describes a vertex-colored 2D vertex
     * @note Attributes a_Position, a_Color
     */
    struct Vertex2Color
    {
        float x, y; // position xy
        union {
            struct { float r, g, b, a; }; // color
            Color rgba;
        };
        static VertexDescr layout() {
            return { sizeof(Vertex2Color), a_Position,2, a_Color,4 };
        }
    };

    /**
     * @note Describes a vertex-alpha 2D vertex
     * @note Attributes a_Vertex with XY position and Z as alpha
     */
    struct Vertex2Alpha
    {
        float x, y; // position xy
        float a;    // alpha color
        static VertexDescr layout() {
            return { sizeof(Vertex2Alpha), a_Vertex,3, };
        }
    };

    /**
     * @note Describes a simple 3D vertex with only position attribute
     * @note Attribute a_Position
     */
    struct Vertex3 // basic 3D vertex
    {
        float x, y, z; // position xyz
        static VertexDescr layout() {
            return{ sizeof(Vertex3), a_Position,3 };
        }
    };

    /**
     * @note Describes a 3D vertex with UV coordinates
     * @note Attributes a_Position, a_Coord
     */
    struct Vertex3UV // basic 3D vertex with UV coordinates
    {
        float x, y, z; // position xyz
        float u, v;    // texture coordinates
        static VertexDescr layout() {
            return { sizeof(Vertex3UV), a_Position,3, a_Coord,2 };
        }
    };

    /**
    * @note Describes the most classic 3D vertex with Position, UV coordinates and vertex normals
    * @note Attributes a_Position, a_Coord, a_Normal
    */
    struct Vertex3UVNorm // basic 3D vertex with UV coordinates
    {
        float x, y, z; // position xyz
        float u, v;    // texture coordinates
        float nx, ny, nz; // vertex normal
        static VertexDescr layout() {
            return { sizeof(Vertex3UVNorm), a_Position,3, a_Coord,2, a_Normal,3 };
        }
    };


    /**
     * @note Describes a vertex-colored 3D vertex
     * @note Attributes a_Position, a_Color
     */
    struct Vertex3Color
    {
        float x, y, z;    // position xyz
        float r, g, b, a; // color rgba

        static VertexDescr layout() {
            return { sizeof(Vertex3Color), a_Position,3, a_Color,4 };
        }
        void set(Vector3 pos, Color color)
        {
            *(Vector3*)&x = pos;
            *(Color*)&r = color;
        }
        void set(float x, float y, float z, Color color)
        {
            this->x = x; this->y = y; this->z = z;
            *(Color*)&r = color;
        }
    };

    /**
     * @note Describes a vertex-alpha 3D vertex
     * @note Attributes a_Vertex with XYZ position and W as alpha
     */
    struct Vertex3Alpha
    {
        float x, y, z; // position xyz
        float a;       // alpha color
        static VertexDescr layout() {
            return { sizeof(Vertex3Alpha), a_Vertex,4, };
        }
    };


    ////////////////////////////////////////////////////////////////////////////////
    

    /** @brief Describes the supported draw modes of our Vertex Buffers */
    enum DrawMode
    {
        DrawNone,          // uninitialized buffer
        DrawIndexed,       // triangles are determined by 3 indices per triangle
        DrawTriangles,     // simple triangle pairs, 3 verts per triangle
        DrawTriangleStrip, // optimized triangle strip, ~1.5 verts per triangle (Google it)
        DrawLines,         // each vertex pair is a unique line
        DrawLineStrip,     // adjacent vertices are lines. if you pass n vertices, you will get n-1 lines
        DrawLineLoop,      // as line strips, except that the first and last vertices are also used as a line
    };


    /** @brief Provides methods for creating efficient Vertex array objects and rendering them */
    class VertexBuffer
    {
        DrawMode drawMode    = DrawNone; // how do we draw the vertices in VAO?
        uint32_t vertexArray = 0;        // vertex array object, required by modern opengl
        int32_t vertexCount  = 0;        // # of verts
        int32_t indexCount   = 0;        // # of indices
        VertexDescr layout;              // vertex layout descriptor

    public:

        VertexBuffer() noexcept;
        ~VertexBuffer();

        VertexBuffer(VertexBuffer&& v) noexcept :
            drawMode    {v.drawMode},
            vertexArray {v.vertexArray},
            vertexCount {v.vertexCount},
            indexCount  {v.indexCount},
            layout      {v.layout}
        {
            v.drawMode    = DrawNone;
            v.vertexArray = 0;
            v.vertexCount = 0;
            v.indexCount  = 0;
            v.layout      = {};
        }
        VertexBuffer& operator=(VertexBuffer&& v) noexcept
        {
            swap(drawMode,    v.drawMode   );
            swap(vertexArray, v.vertexArray);
            swap(vertexCount, v.vertexCount);
            swap(indexCount,  v.indexCount );
            swap(layout,      v.layout     );
            return *this;
        }

        VertexBuffer(const VertexBuffer&) = delete; // NOCOPY
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        bool empty() const { return vertexCount == 0; }
        explicit operator bool() const { return vertexCount != 0; }
        bool     operator!    () const { return vertexCount == 0; }
        
        bool operator==(const VertexBuffer& v) const noexcept
        {
            return this == &v || memcmp(this, &v, sizeof(*this)) == 0;
        }
        bool operator!=(const VertexBuffer& v) const noexcept
        {
            return this != &v || memcmp(this, &v, sizeof(*this)) != 0;
        }

        // enables vertex attributes according to layout descriptor
        void enableAttribs();
        void disableAttribs();

        /**
         * Creates a new VBO to store a vertex element array (DrawIndexed)
         * @param verts      Pointer to vertex data
         * @param numVerts   Number of vertices, each sizeOf bytes
         * @param indices    Pointer to index data
         * @param numIndices Number of indices
         * @param layout     Vertex layout descriptor
         *
         * @code
         *    struct Vertex3UV { vec3 pos; vec2 tex; };
         *    VertexDescr vd = { sizeof(Vertex3UV), a_Position,3, a_Coord,2 };
         *    buf.create(verts, nverts, indices, nindices, vd);
         * @endcode
         */
        void create(const void*    verts,   int numVerts,
                    const index_t* indices, int numIndices,
                    const VertexDescr& layout);

        // automatic template wrapper, requires static layout() provider
        template<class VERTEX>
        void create(const VERTEX* verts, int numVerts, 
                    const index_t* indices, int numIndices)
        {
            create(verts, numVerts, indices, numIndices, VERTEX::layout());
        }

        template<class VERTEX>
        void create(const vector<VERTEX>& verts, const vector<index_t>& indices)
        {
            create(verts.data(), (int)verts.size(), indices.data(), (int)indices.size(), VERTEX::layout());
        }

        /**
         * Creates a new VBO to store a vertex array
         * @param mode       DrawMode to use: DrawTriangles or DrawTriangleStrip
         * @param verts      Pointer to vertex data
         * @param numVerts   Number of vertices, each sizeOf bytes
         * @param layout     Vertex layout descriptor
         * @code
         *    struct Vertex3UV { Vector3 pos; Vector2 tex; };
         *    VertexDescr vd = { sizeof(Vertex3UV), a_Position,3, a_Coord,2 };
         *    buf.create(DrawTriangles, verts, nverts, vd);
         * @endcode
         */
        void create(DrawMode mode, const void* verts, 
                    int numVerts, const VertexDescr& layout);
        
        // automatic template wrapper, requires static layout() provider
        template<class VERTEX> 
        void create(DrawMode mode, const VERTEX* verts, int numVerts)
        {
            create(mode, verts, numVerts, VERTEX::layout());
        }

        template<class VERTEX, int NUMVERTS>
        void create(DrawMode mode, const VERTEX (&verts)[NUMVERTS])
        {
            create(mode, verts, NUMVERTS, VERTEX::layout());
        }

        template<class VERTEX>
        void create(DrawMode mode, const vector<VERTEX>& verts)
        {
            create(mode, verts.data(), (int)verts.size());
        }

        // draws the vertices; make sure you have bound a shader with some uniforms beforehand
        void draw() const;

        // destroys all buffers and restores default empty state
        void clear();

        /**
         * @return TRUE if `VertexDescr` layout responds to the given attribute
         */
        bool hasAttrib(ShaderAttr shaderAttr) const;
    };


    ////////////////////////////////////////////////////////////////////////////////

    class Shader
    {
        uint32_t program;  // linked glProgram
        string vsPath;     // vert shader path
        string fsPath;     // frag shader path
        time_t vsMod;      // last modified time of vert shader file
        time_t fsMod;      // last modified time of frag shader file
        char uniforms  [u_MaxUniforms]; // uniform locations
        bool attributes[a_MaxAttributes] = {};  // attribute present? true/false

    public:
        /** @brief Default initializes this shader object */
        Shader();
        ~Shader();
        
        Shader(Shader&& s)            = default; // Allow MOVE
        Shader& operator=(Shader&& s) = default;
        Shader(const Shader& s)            = delete; // NO COPY
        Shader& operator=(const Shader& s) = delete;
        
        explicit operator bool() const { return program != 0; }
        bool operator!()const { return !program; }

        bool operator==(const Shader& s) const {
            return vsPath == s.vsPath && fsPath == s.fsPath;
        }
        bool operator!=(const Shader& s) const {
            return vsPath != s.vsPath || fsPath != s.fsPath;
        }

        string name() const;
        const string& vertShader() const { return vsPath; }
        const string& fragShader() const { return fsPath; }

        /** @return TRUE if the GLSL program has linked */
        bool good() const { return program != 0; }
        /** @brief Loads shader from {shaderName}.frag and {shaderName}.vert */
        bool loadShader(const string& shaderName);
        /** @brief Reloads shader if VS or FS are modified. */
        bool hotload();
        /** @brief Forces a full recompile of the shaders */
        bool reload();
        /** 
         * @brief Unloads and deletes the shader program
         * @note However, the shader can be reloaded with reload()
         */
        void unload();
    private:
        void loadUniforms();
        void checkUniform(const char* where, ShaderUniform uniformSlot) const;

    public:
        /** @brief Binds the shader program for rendering */
        void bind();
        /** @brief Unbinds the shader */
        void unbind();

        /** @return TRUE if the specified uniform is active */
        bool activeUniform(ShaderUniform uniformSlot) const;

        /** @return TRUE if the specified attribute is active */
        bool activeAttrib(ShaderAttr attrSlot) const;

        void bind(ShaderUniform uniformSlot, const Matrix4& matrix);
        void bind(ShaderUniform uniformSlot, unsigned glTexture);
        void bind(ShaderUniform uniformSlot, const Texture& texture);
        void bind(ShaderUniform uniformSlot, const Texture* texture);
        void bind(ShaderUniform uniformSlot, const Vector2& value);
        void bind(ShaderUniform uniformSlot, const Vector3& value);
        void bind(ShaderUniform uniformSlot, const Vector4& value);
    };
    
    ////////////////////////////////////////////////////////////////////////////
    
    /**
     * @brief Combines a Shader with additional texture and color information
     */
    struct Material
    {
        /** @brief WEAK REF to a shader used by this material */
        Shader* shader;

        /** @brief WEAK REF to a texture used during rendering */
        TextureRef texture;
        
        /** @brief Color to be used as u_DiffuseColor uniform param (default is white) */
        Color color;

        /** Color to be used as u_OutlineColor uniform param (default is invisible) */
        Color border;
        
        Material(Shader* s = nullptr, const TextureRef& t = TextureRef{},
                 Color c = Color::White(), Color b = Color::Zero())
                : shader{s}, texture{t}, color{c}, border{b}
        {
        }

        TextureRef* operator->() { return &texture; }
        
        explicit operator bool() const { return  shader &&  texture; }
        bool operator!()         const { return !shader || !texture; }

        bool operator==(const Material& m) const {
            return shader == m.shader && texture == m.texture
                && color  == m.color  && border  == m.border;
        }
        bool operator!=(const Material& m) const {
            return shader != m.shader || texture != m.texture
                || color  != m.color  || border  != m.border;
        }
        
        /** @return TRUE if the underlying TextureRef is loading from remote source */
        bool isLoading()  const { return texture.isLoading(); }
        /** @return TRUE if the GL texture can be bound for rendering */
        bool isBindable() const { return texture.isBindable(); }
        
        void set(Shader* s, const TextureRef& t, Color c, Color o)
        {
            shader  = s;
            texture = t;
            color   = c;
            border  = o;
        }
    };
    
    ////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
