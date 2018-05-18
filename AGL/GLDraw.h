#pragma once
#include "Shader.h"
#include <rpp/collections.h>

namespace AGL
{
    ////////////////////////////////////////////////////////////////////////////////

    /** 
     * Utility class for generating simple 2D geometric vertex buffers
     *
     * @note SHADER INFORMATION:
     *     Vertex2Alpha is used, meaning lines have position
     *     and alpha (smooth edges).
     *     If you need extra color, your shader must use uniform diffuseColor
     *     Check shaders/vertexalpha.vert
     */
    class GLDraw2D
    {
        vector<Vertex2Alpha> vertices;
        vector<index_t>      indices;

    public:

        /**
         * @note Creates a VertexBuffer based on the current state
         */
        void CreateBuffer(VertexBuffer& outBuffer) const;

        /**
         * @note Creates a VertexBuffer based on the current state
         * @return A new VertexBuffer
         */
        VertexBuffer CreateBuffer() const;

        /**
         * @note Clears the current drawing
         */
        void Clear();

        /**
         * Draws a simple jagged line. Not good for most general usage, but ok for debugging.
         */
        void Line(const Vector2& p1, const Vector2& p2, const float width = 1.0f);

        /**
         * @note Draws an anti-aliased line
         * @param p1 Starting point of the line
         * @param p2 Ending point of the line
         * @param width Width of the line
         */
        void LineAA(const Vector2& p1, const Vector2& p2, const float width = 1.0f);

        /**
         * @note Creates a rectangle with AA lines
         * @param origin Lower left corner of the rectangle - the origin
         * @param size Width & Height of the rectangle
         * @param width Width of the lines to draw
         */
        void RectAA(const Vector2& origin, const Vector2& size, float width = 1.0f);

        /**
         * @note Creates a circle with AA lines
         * @param center Center of the circle 
         * @param radius Radius of the circle
         * @param width Width of the lines to draw
         */
        void CircleAA(const Vector2& center, float radius, float width = 1.0f);

        /**
         * @note Fills a rectangle
         * @param origin Lower left corner of the rectangle - the origin
         * @param size Width & Height of the rectangle
         */
        void FillRect(const Vector2& origin, const Vector2& size);

        // calculates the left normal between 3 points
        static Vector2 Normal(const Vector2& p1, const Vector2& p2, const Vector2& p3);
        static Vector2 StartNormal(const Vector2 points[], const int count);
        template<int N> static Vector2 StartNormal(const Vector2 (&points)[N])
        {
            return StartNormal(points, N);
        }
        static Vector2 EndNormal(const Vector2 points[], const int count);
        template<int N> static Vector2 EndNormal(const Vector2(&points)[N])
        {
            return EndNormal(points, N);
        }

        // calculates the normal offset point between 3 array points centered at index
        static Vector2 LeftNormalPoint(const int centerIndex, float edgeOffset, 
                                       const Vector2 points[], const int count);

        template<int N> static Vector2 LeftNormalPoint(const int centerIndex, float edgeOffset, 
                                                       const Vector2(&points)[N])
        {
            return LeftNormalPoint(centerIndex, edgeOffset, points, N);
        }

        /**
         * Fills a symmetric shape with triangles and provides AA line edges
         * @note The points must progress from LEFT to RIGHT (Clockwise)!
         * @note The shape must be symmetric! Otherwise triangle pairs will be skewed!
         *
         * @param points Shape points
         * @param count  Number of shape points
         * @param edgeWidth Width of the AA edges
         */
        void FillSymmetricShapeAA(const Vector2 points[], const int count, float edgeWidth = 1.0f);
        template<int N> void FillSymmetricShapeAA(const Vector2 (&points)[N], float edgeWidth = 1.0f)
        {
            FillSymmetricShapeAA(points, N, edgeWidth);
        }
        
        /**
         * Creates several AA lines and connects the last and first point
         * @param points Line Shape points
         * @param count Number of points
         * @param lineWidth Width of the line shape
         */
        void LineShapeAA(const Vector2 points[], const int count, float lineWidth = 1.0f);
        template<int N> void LineShapeAA(const Vector2 (&points)[N], float lineWidth = 1.0f)
        {
            LineShapeAA(points, N, lineWidth);
        }

        /**
         * Creates several standard jagged lines and connects the last and first point
         */
        void LineShape(const Vector2 points[], const int count, float lineWidth = 1.0f);
        template<int N> void LineShape(const Vector2 (&points)[N], float lineWidth = 1.0f)
        {
            LineShape(points, N, lineWidth);
        }
        
    };

    ////////////////////////////////////////////////////////////////////////////////

    /**
     * Base class for debug primitives
     */
    class DebugPrimitive
    {
    protected:
        VertexBuffer verts;

    public:
        void Draw(Shader* shader, const Matrix4& modelViewProjection, const Color& color);
    };


    /**
     * A simple Anti-Aliased debug rect
     */
    class DebugRect : public DebugPrimitive
    {
        Rect rect;

    public:
        DebugRect() : rect(Rect::ZERO) {}
        void SetRect(const Rect& r);
    };

    
    /**
     * A simple Anti-Aliased debug circle
     */
    class DebugCircle : public DebugPrimitive
    {
        Vector2 point;
        float radius;

    public:
        DebugCircle() : point(Vector2::ZERO), radius(1.0f) {}

        void SetCircle(Vector2 point, float radius);
    };

    ////////////////////////////////////////////////////////////////////////////////

    class GLDraw3D
    {
        vector<Vertex3Color> vertices;
        vector<index_t> indices;

    public:

        /**
         * @note Creates a VertexBuffer based on the current state
         */
        void CreateBuffer(VertexBuffer& outBuffer) const;

        /**
         * @note Creates a VertexBuffer based on the current state
         * @return A new VertexBuffer
         */
        VertexBuffer CreateBuffer() const;

        static VertexBuffer CreatePoints(rpp::element_range<const Vector3> points, float radius, Color color);

        void Append(const GLDraw3D& draw);

        /**
         * Calls Point for each element in `points` with the given radius
         */
        void Points(rpp::element_range<const Vector3> points, float radius, Color color);

        /**
         * Clears the current drawing
         */
        void Clear();

        /**
         * Reserve memory to make shape creation faster
         */
        void Reserve(int newVertices, int newTriangles);

        /**
         * Reserves enough memory to draw `numLines`
         */
        void ReserveLines(int numLines);

        /**
         * Draws a simple jagged line. Not good for most general usage, but ok for debugging.
         *     2
         *    /|\
         *   0---1     
         *   | | |
         *   | 5 |
         *   |/ \|
         *   3---4
         */
        void Line(Vector3 a, Vector3 b, float lineWidth, Color colorA, Color colorB);
        void Line(Vector3 a, Vector3 b, float lineWidth, Color color);

        /**
         * Draws a 3D box. Good for very basic 3D visualizations
         *       4------7
         *      /|     /|
         * max 0------3 |
         *     | 5----|-6 min
         *     |/     |/
         *     1------2
         */
        void Box(Vector3 min, Vector3 max, Color color);
        
        /**
         * Draws a simple cube. Good for very basic 3D visualizations
         *     4------7
         *    /|     /|
         *   0------3 |
         *   | 5----|-6
         *   |/     |/
         *   1------2
         */
        void Cube(Vector3 center, float radius, Color color);
        
        /**
         * Draws a more complex hollow box using lines
         */
        void HollowBox(Vector3 min, Vector3 max, float lineWidth, Color color);

        /**
         * Draws a more complex hollow cube using lines
         */
        void HollowCube(Vector3 center, float radius, float lineWidth, Color color);

        /**
         * Draws a simple prism consisting of 6 points
         *        0
         *      .` \`.
         *    .` 4  \  `.3
         *   1_______2.`.
         *    `.    / .
         *      `. /.
         *        5
         */
        void Prism(Vector3 center, float radius, Color color);

        /**
         * Draws a simple jagged point. Not good for most general usage, but ok for debugging.
         */
        void Point(Vector3 p, float radius, Color color);

        /**
         * Draws a 3D cylinder from point A to point B with color and radius
         */
        void Cylinder(Vector3 a, Vector3 b, float radius, Color color);

        /*
         * Draws a more complex hollow cylinder using lines
         */
        void HollowCylinder(Vector3 a, Vector3 b, float radius, float lineWidth, Color color);
        void HollowCylinder(Vector3 a, Vector3 b, float radius, float lineWidth, int segments, Color color);

        void Sphere(Vector3 center, float radius, Color color);
        void HollowSphere(Vector3 center, float radius, Color color, float lineWidth);

    };

    ////////////////////////////////////////////////////////////////////////////////
}
