#include "GLDraw.h"
#include <rpp/debugging.h>
#include <cmath>

namespace AGL
{
    ////////////////////////////////////////////////////////////////////////////////

    void GLDraw2D::CreateBuffer(VertexBuffer& outBuffer) const
    {
        if (!vertices.empty())
            outBuffer.create<Vertex2Alpha>(vertices, indices);
        else outBuffer.clear(); // only clear if verts is 0, to avoid free/malloc cycle
    }

    VertexBuffer GLDraw2D::CreateBuffer() const
    {
        VertexBuffer buf;
        if (!vertices.empty())
            buf.create<Vertex2Alpha>(vertices, indices);
        return buf;
    }

    void GLDraw2D::Clear()
    {
        vertices.clear();
        indices.clear();
    }

    void GLDraw2D::Line(const Vector2& p1, const Vector2& p2, const float lineWidth)
    {
        //  0---3   0\``3
        //  | + |   | \ |
        //  1---2   1--\2
        float x1 = p1.x, x2 = p2.x;
        float y1 = p1.y, y2 = p2.y;
        const Vector2 left = p1.left(p2, lineWidth * 0.5f);

        float cx = left.x, cy = left.y; // center xy offsets

        index_t n = (index_t)vertices.size();
        vertices.resize(n + 4);
        Vertex2Alpha* v = &vertices[n];
        v[0].x = x1 - cx, v[0].y = y1 - cy, v[0].a = 1.0f; // left-top
        v[1].x = x2 - cx, v[1].y = y2 - cy, v[1].a = 1.0f; // left-bottom
        v[2].x = x2 + cx, v[2].y = y2 + cy, v[2].a = 1.0f; // right-bottom
        v[3].x = x1 + cx, v[3].y = y1 + cy, v[3].a = 1.0f; // right-top

        size_t numIndices = indices.size();
        indices.resize(numIndices + 6);
        index_t* i = &indices[numIndices];
        i[0] = n, i[1] = n + 1, i[2] = n + 2;	// triangle 1
        i[3] = n, i[4] = n + 2, i[5] = n + 3;	// triangle 2
    }
    
    // core radius determines the width of the line core
    // for very small widths, the core should be very small ~10%
    // for large width, the core should be very large ~90%
    static void lineCoreRadii(const float width, float& cr, float& w2)
    {
        switch ((int)width) {
            case 0:
            case 1:  w2 = (width + 0.5f) * 0.5f; cr = 0.25f; return;
            case 2:  w2 = width * 0.5f; cr = 0.75f; return;
            case 3:  w2 = width * 0.5f; cr = 1.5f;  return;
            // always leave 1 pixel for the edge radius
            default: w2 = width * 0.5f; cr = w2 - 1.0f; return;
        }
    }
    
    void GLDraw2D::LineAA(const Vector2& p1, const Vector2& p2, const float width)
    {
        // 12 vertices
        //      x1                A up     
        // 0\``2\``4\``6    left  |  right 
        // | \ | \ | \ |    <-----o----->  
        // 1__\3__\5__\7          |         
        //      x2                V down
        
        float cr, w2;
        lineCoreRadii(width, cr, w2);

        float x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y;
        Vector2 right { y2 - y1, x1 - x2 };
        right.normalize();

        // extend start and end by a tiny amount (core radius to be exact)
        Vector2 dir { x2 - x1, y2 - y1 };
        dir.normalize(cr);
        x1 -= dir.x;
        y1 -= dir.y;
        x2 += dir.x;
        y2 += dir.y;

        float ex = right.x * w2, ey = right.y * w2; // edge xy offsets
        float cx = right.x * cr, cy = right.y * cr; // center xy offsets
        index_t n = (index_t)vertices.size();
        vertices.resize(n + 8);
        Vertex2Alpha* v = &vertices[n];
        v[0].x = x1 - ex, v[0].y = y1 - ey, v[0].a = 0.0f;	// left-top
        v[1].x = x2 - ex, v[1].y = y2 - ey, v[1].a = 0.0f;	// left-bottom
        v[2].x = x1 - cx, v[2].y = y1 - cy, v[2].a = 1.0f;	// left-middle-top
        v[3].x = x2 - cx, v[3].y = y2 - cy, v[3].a = 1.0f;	// left-middle-bottom
        v[4].x = x1 + cx, v[4].y = y1 + cy, v[4].a = 1.0f;	// right-middle-top
        v[5].x = x2 + cx, v[5].y = y2 + cy, v[5].a = 1.0f;	// right-middle-bottom
        v[6].x = x1 + ex, v[6].y = y1 + ey, v[6].a = 0.0f;	// right-top
        v[7].x = x2 + ex, v[7].y = y2 + ey, v[7].a = 0.0f;	// right-bottom

        size_t numIndices = indices.size();
        indices.resize(numIndices + 18);
        index_t* i = &indices[numIndices];
        i[0]  = n + 0, i[1]  = n + 1, i[2]  = n + 3; // triangle 1
        i[3]  = n + 0, i[4]  = n + 3, i[5]  = n + 2; // triangle 2
        i[6]  = n + 2, i[7]  = n + 3, i[8]  = n + 5; // triangle 3
        i[9]  = n + 2, i[10] = n + 5, i[11] = n + 4; // triangle 4
        i[12] = n + 4, i[13] = n + 5, i[14] = n + 7; // triangle 5
        i[15] = n + 4, i[16] = n + 7, i[17] = n + 6; // triangle 6
    }

    void GLDraw2D::RectAA(const Vector2& origin, const Vector2& size, float lineWidth)
    {
        //  0---3
        //  | + |
        //  1---2
        Vector2 p0{ origin.x, origin.y };
        Vector2 p1{ origin.x, origin.y + size.y };
        Vector2 p2{ origin.x + size.x, origin.y + size.y };
        Vector2 p3{ origin.x + size.x, origin.y };
        LineAA(p0, p1, lineWidth);
        LineAA(p1, p2, lineWidth);
        LineAA(p2, p3, lineWidth);
        LineAA(p3, p0, lineWidth);
    }

    void GLDraw2D::CircleAA(const Vector2& center, float radius, float lineWidth)
    {
        // adaptive line count
        const int   segments   = 12 + (int(radius) / 6);
        const float segmentArc = (2.0f * rpp::PIf) / segments;
        const float x = center.x, y = center.y;

        float alpha = segmentArc;
        Vector2 A { x, y + radius };
        for (int i = 0; i < segments; ++i)
        {
            Vector2 B { x + sinf(alpha)*radius, y + cosf(alpha)*radius };
            LineAA(A, B, lineWidth);
            A = B;
            alpha += segmentArc;
        }
    }

    void GLDraw2D::FillRect(const Vector2& origin, const Vector2& size)
    {
        //  0---3   0\``3
        //  | + |   | \ |
        //  1---2   1--\2
        float x1 = origin.x, x2 = origin.x + size.x;
        float y1 = origin.y, y2 = origin.y + size.y;
        index_t n = (int)vertices.size();
        vertices.resize(size_t(n + 4));
        Vertex2Alpha* v = &vertices[n];
        v[0].x = x1, v[0].y = y2, v[0].a = 1.0f; // left-top
        v[1].x = x1, v[1].y = y1, v[1].a = 1.0f; // left-bottom
        v[2].x = x2, v[2].y = y1, v[2].a = 1.0f; // right-bottom
        v[3].x = x2, v[3].y = y2, v[3].a = 1.0f; // right-top

        size_t numIndices = indices.size();
        indices.resize(numIndices + 6);
        index_t* i = &indices[numIndices];
        i[0] = n, i[1] = n + 1, i[2] = n + 2;	// triangle 1
        i[3] = n, i[4] = n + 2, i[5] = n + 3;	// triangle 2
    }
    
    Vector2 GLDraw2D::Normal(const Vector2& p1, const Vector2& p2, const Vector2& p3)
    {
        const Vector2 dirA = (p1 - p2).left();
        const Vector2 dirB = (p2 - p3).left();
        return (dirA + dirB).normalized();
    }
    Vector2 GLDraw2D::StartNormal(const Vector2 points[], const int count)
    {
        Assert(count >= 3, "Normal calculation requires at least 3 points");
        return Normal(points[count - 1], points[0], points[1]);
    }
    Vector2 GLDraw2D::EndNormal(const Vector2 points[], const int count)
    {
        Assert(count >= 3, "Normal calculation requires at least 3 points");
        return Normal(points[count - 2], points[count - 1], points[0]);
    }

    Vector2 GLDraw2D::LeftNormalPoint(const int centerIndex, float edgeOffset, const Vector2 points[], const int count)
    {
        Assert(count >= 3, "Normal calculation requires at least 3 points");

        int idx1 = centerIndex > 0 ? centerIndex - 1 : count - 1;
        int idx2 = centerIndex < count - 1 ? centerIndex + 1 : 0;

        const Vector2& center = points[centerIndex];
        const Vector2 normalA = (points[idx1] - center).left();
        const Vector2 normalB = (center - points[idx2]).left();

        // @todo Find a proper solution for this.. right now it's Good Enough (TM)
        // detect 90 degree corners
        float corner = normalA.dot(normalB) == 0.0f ? rpp::SQRT2f : 1.0f;

        const Vector2 offset = (normalA + normalB).normalized(corner * edgeOffset);
        return center + offset;
    }

    void GLDraw2D::FillSymmetricShapeAA(const Vector2 points[], const int count, float edgeWidth)
    {
        Assert(count % 2 == 0, "Shape count(%d) is not symmetric!", count);
        
        const int nquads = (count / 2) - 1; // number of quads produced

        vertices.reserve(vertices.size() + count * 2);
        indices.reserve(indices.size() + count*6 + nquads*6);
        
        // interleave edge vertices with regular points
        //  1-------3
        //  |\     / 
        //  | 0---2-
        //  | |   |
        const index_t n = (index_t)vertices.size();
        const index_t n2 = n + count * 2;
        vertices.resize(n2);
        Vertex2Alpha* vtx = &vertices[n];

        for (int i = 0; i < count; ++i, vtx += 2)
        {
            const Vector2& pt0 = points[i];
            Vector2 pt1 = LeftNormalPoint(i, edgeWidth, points, count);
            vtx[0] = { pt0.x, pt0.y, 1.0f };
            vtx[1] = { pt1.x, pt1.y, 0.0f };
        }
        
        // first form the edge faces, because they're easier
        size_t m = indices.size();
        indices.resize(m + count*6);
        index_t* idx = &indices[m];
        index_t* eidx = idx + count * 6;
        for (index_t ib = n; idx != eidx; idx += 6, ib += 2)
        {
            index_t ib2 = ib + 2;
            index_t ib3 = ib + 3;
            if (ib2 >= n2) ib2 = n;
            if (ib3 >= n2) ib3 = n+1; // wrap the final index to close the shape loop

            idx[0] = ib+1, idx[1] = ib,  idx[2] = ib2;	// triangle 1
            idx[3] = ib2,  idx[4] = ib3, idx[5] = ib+1;	// triangle 2
        }

        // now fill the shape. since the shape is assumed to be symmetrical,
        // we connect with opposite vertices
        //  |\     /|
        //  | 0---2 |
        //  | | + | |
        //  | 6---4 |
        //  |/     \|
        m = indices.size();
        indices.resize(m + nquads * 6);
        idx  = &indices[m];
        eidx = idx + nquads * 6;

        for (index_t i = 0; idx != eidx; idx += 6, i += 2)
        {
            index_t ib = n + i;
            index_t ie = n + count * 2 - 2 - i;

            idx[0] = ib, idx[1] = ie,   idx[2] = ie-2;	// triangle 1
            idx[3] = ib, idx[4] = ie-2, idx[5] = ib+2;	// triangle 2
        }
    }
    
    void GLDraw2D::LineShapeAA(const Vector2 points[], const int count, float lineWidth)
    {
        vertices.reserve(vertices.size() + count * 8);
        
        for (int i = 1; i < count; ++i) {
            LineAA(points[i-1], points[i], lineWidth);
        }
        if (count > 2) {
            LineAA(points[count-1], points[0], lineWidth);
        }
    }

    void GLDraw2D::LineShape(const Vector2 points[], const int count, float lineWidth)
    {
        vertices.reserve(vertices.size() + count * 4);

        for (int i = 1; i < count; ++i) {
            Line(points[i - 1], points[i], lineWidth);
        }
        if (count > 2) {
            Line(points[count - 1], points[0], lineWidth);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////


    void DebugPrimitive::Draw(Shader* shader, const Matrix4& modelViewProjection, const Color& color)
    {
        Assert(shader != nullptr, "Shader cannot be null");
        shader->bind();
        shader->bind(u_Transform, modelViewProjection);
        shader->bind(u_DiffuseColor, color);
        verts.draw();
    }

    void DebugRect::SetRect(const Rect& r)
    {
        if (rect != r)
        {
            rect = r;

            GLDraw2D draw;
            draw.RectAA(r.pos, r.size, 1.5f);
            draw.CreateBuffer(verts);
        }
    }

    void DebugCircle::SetCircle(Vector2 p, float r)
    {
        if (point != p || radius != r)
        {
            point  = p;
            radius = r;
            GLDraw2D draw;
            draw.CircleAA(p, r, 1.5f);
            draw.CreateBuffer(verts);
        }
    }


    ////////////////////////////////////////////////////////////////////////////////

    void GLDraw3D::CreateBuffer(VertexBuffer& outBuffer) const
    {
        if (!vertices.empty())
            outBuffer.create<Vertex3Color>(vertices, indices);
        else outBuffer.clear(); // only clear if verts is 0, to avoid free/malloc cycle
    }

    VertexBuffer GLDraw3D::CreateBuffer() const
    {
        VertexBuffer buf;
        if (!vertices.empty())
            buf.create<Vertex3Color>(vertices, indices);
        return buf;
    }

    VertexBuffer GLDraw3D::CreatePoints(rpp::element_range<const Vector3> points, float radius, Color color)
    {
        GLDraw3D draw;
        draw.Points(points, radius, color);
        return draw.CreateBuffer();
    }

    void GLDraw3D::Append(const GLDraw3D& draw)
    {
        size_t offset = vertices.size();
        vertices.insert(vertices.end(), draw.vertices.begin(), draw.vertices.end());

        size_t updateStart = indices.size();
        indices.insert(indices.end(), draw.indices.begin(), draw.indices.end());

        size_t numIndices = indices.size();
        index_t* indexPtr = indices.data();
        for (size_t i = updateStart; i < numIndices; ++i)
            indexPtr[i] += offset;
    }

    void GLDraw3D::Points(rpp::element_range<const Vector3> points, float radius, Color color)
    {
        for (const Vector3& pt : points)
            Point(pt, radius, color);
    }

    void GLDraw3D::Clear()
    {
        vertices.clear();
        indices .clear();
    }

    void GLDraw3D::Reserve(int newVertices, int newTriangles)
    {
        vertices.reserve(vertices.size() + newVertices);
        indices .reserve(indices.size()  + newTriangles*6);
    }

    void GLDraw3D::ReserveLines(int numLines)
    {
        constexpr int vertsPerLine = 6;
        constexpr int trisPerLine = 8;
        Reserve(numLines*vertsPerLine, numLines*trisPerLine);
    }

    void GLDraw3D::Line(Vector3 a, Vector3 b, float lineWidth, Color color)
    {
        Line(a, b, lineWidth, color, color);
    }

    void GLDraw3D::Line(Vector3 a, Vector3 b, float lineWidth, Color colorA, Color colorB)
    {
        //     2
        //    /|\
        //   0---1     
        //   | | |
        //   | 5 |
        //   |/ \|
        //   3---4
        index_t n = (index_t)vertices.size();
        vertices.resize(n + 6);

        Vector3 ab = b - a;
        Vector3 localZ = ab.normalized(); // Z dir
        Vector3 localX = localZ.cross((localZ + Vector3::One()).normalized());
        Vector3 localY = localZ.cross(localX);

        float r = lineWidth * 0.5f;
        Vector3 p0 = a - r*(localX + localY);
        Vector3 p1 = a + r*(localX - localY);
        Vector3 p2 = a + r*localY;
        Vertex3Color* v = (Vertex3Color*)&vertices[n];
        v[0].set(p0, colorA);
        v[1].set(p1, colorA);
        v[2].set(p2, colorA);
        v[3].set(p0 + ab, colorB);
        v[4].set(p1 + ab, colorB);
        v[5].set(p2 + ab, colorB);

        size_t numIndices = indices.size();
        indices.resize(numIndices + 24); // 6 per side, 3 per cap
        index_t* i = &indices[numIndices];

        auto addFace = [&](int k, int l, int m) {
            i[0] = n + k, i[1] = n + l, i[2] = n + m; // triangle 1: 012
            i += 3;
        };
        addFace(0, 1, 2); // top cap
        addFace(0, 3, 4); addFace(0, 4, 1);
        addFace(1, 4, 5); addFace(1, 5, 2);
        addFace(2, 5, 3); addFace(2, 3, 0);
        addFace(3, 5, 4); // bot cap
    }

    void GLDraw3D::Box(Vector3 min, Vector3 max, Color color)
    {
        //       4------7
        //      /|     /|
        // max 0------3 |
        //     | 5----|-6 min
        //     |/     |/
        //     1------2
        index_t n = (index_t)vertices.size();
        vertices.resize(n + 8);

        Vertex3Color* v = (Vertex3Color*)&vertices[n];
        v[0].set(max, color);
        v[1].set(max.x, min.y, max.z, color);
        v[2].set(min.x, min.y, max.z, color);
        v[3].set(min.x, max.y, max.z, color);
        v[4].set(max.x, max.y, min.z, color);
        v[5].set(max.x, min.y, min.z, color);
        v[6].set(min, color);
        v[7].set(min.x, max.y, min.z, color);

        size_t numIndices = indices.size();
        indices.resize(numIndices + 6*6); // 6 indices per side
        index_t* i = &indices[numIndices];
        
        auto addSide = [&](int a, int b, int c, int d) {
            i[0] = n+a, i[1] = n+b, i[2] = n+c; // triangle 1: 012
            i[3] = n+a, i[4] = n+c, i[5] = n+d; // triangle 2: 023
            i += 6;
        };
        addSide(0, 1, 2, 3);
        addSide(3, 2, 6, 7);
        addSide(0, 3, 7, 4);
        addSide(4, 5, 1, 0);
        addSide(7, 6, 5, 4);
        addSide(1, 5, 6, 2);
    }

    void GLDraw3D::Cube(Vector3 center, float radius, Color color)
    {
        Vector3 offset = { radius, radius, radius };
        Box(center - offset, center + offset, color);
    }

    void GLDraw3D::HollowBox(Vector3 min, Vector3 max, float lineWidth, Color color)
    {
        //       4------7
        //      /|     /|
        // max 0------3 |
        //     | 5----|-6 min
        //     |/     |/
        //     1------2
        ReserveLines(12);

        Vector3 p0 = max;
        Vector3 p1 = { max.x, min.y, max.z };
        Vector3 p2 = { min.x, min.y, max.z };
        Vector3 p3 = { min.x, max.y, max.z };
        Vector3 p4 = { max.x, max.y, min.z };
        Vector3 p5 = { max.x, min.y, min.z };
        Vector3 p6 = min;
        Vector3 p7 = { min.x, max.y, min.z };

        Line(p0, p1, lineWidth, color);
        Line(p0, p3, lineWidth, color);
        Line(p0, p4, lineWidth, color);

        Line(p2, p1, lineWidth, color);
        Line(p2, p3, lineWidth, color);
        Line(p2, p6, lineWidth, color);

        Line(p5, p1, lineWidth, color);
        Line(p5, p4, lineWidth, color);
        Line(p5, p6, lineWidth, color);
        
        Line(p7, p3, lineWidth, color);
        Line(p7, p4, lineWidth, color);
        Line(p7, p6, lineWidth, color);
    }
    
    void GLDraw3D::HollowCube(Vector3 center, float radius, float lineWidth, Color color)
    {
        Vector3 offset = { radius, radius, radius };
        HollowBox(center - offset, center + offset, lineWidth, color);
    }

    void GLDraw3D::Prism(Vector3 center, float radius, Color color)
    {
        //        0
        //      .` \`. 
        //    .` 4  \  `.3
        //   1_______2.`.
        //    `.    / .
        //      `. /.
        //        5
        index_t n = (index_t)vertices.size();
        vertices.resize(n + 6);

        const float length = radius * 2;
        Vector3 p1 = center + radius * Vector3{ -1.0f, 0.0f, -1.0f};
        Vertex3Color* v = (Vertex3Color*)&vertices[n];
        v[0].set(center + Vector3::Up()*radius, color);
        v[1].set(p1, color);
        v[2].set(p1 + length * Vector3::Right(), color);
        v[3].set(p1 + length * Vector3{ +1.0f, 0.0f, +1.0f }, color);
        v[4].set(p1 + length * Vector3::Forward(), color);
        v[5].set(center - Vector3::Up()*radius, color);

        size_t numIndices = indices.size();
        indices.resize(numIndices + 3 * 8); // 3 indices per face
        index_t* i = &indices[numIndices];

        auto addFace = [&](int a, int b, int c) {
            i[0] = n + a, i[1] = n + b, i[2] = n + c; // triangle 1: 012
            i += 3;
        };
        addFace(0, 1, 2);
        addFace(0, 2, 3);
        addFace(0, 3, 4);
        addFace(0, 4, 1);
        addFace(5, 2, 1);
        addFace(5, 3, 2);
        addFace(5, 4, 3);
        addFace(5, 1, 4);
    }

    void GLDraw3D::Point(Vector3 p, float radius, Color color)
    {
        Prism(p, radius, color);
    }

    void GLDraw3D::Cylinder(Vector3 a, Vector3 b, float radius, Color color)
    {
        // adaptive line count
        //const int   segments   = 12 + (int(radius) / 6);
        //const float segmentArc = (2.0f * rpp::PIf) / segments;
        //const float x = center.x, y = center.y;

        //float alpha = segmentArc;
        //Vector2 A(x, y + radius);
        //for (int i = 0; i < segments; ++i)
        //{
        //    Vector2 B(x + sinf(alpha)*radius, y + cosf(alpha)*radius);
        //    LineAA(A, B, lineWidth);
        //    A = B;
        //    alpha += segmentArc;
        //}
    }

    void GLDraw3D::HollowCylinder(Vector3 a, Vector3 b, float radius, float lineWidth, Color color)
    {
        // adaptive line count
        const int segments = 12 + (int(radius) / 6);
        HollowCylinder(a, b, radius, lineWidth, segments, color);
    }

    void GLDraw3D::HollowCylinder(Vector3 a, Vector3 b, float radius, float lineWidth, int segments, Color color)
    {
        const float segmentArc = (2.0f * rpp::PIf) / segments;
        ReserveLines(segments*3);

        Vector3 ab = b - a;
        Vector3 localZ = ab.normalized(); // Z dir
        Vector3 localX = localZ.cross((localZ + Vector3::One()).normalized());
        Vector3 localY = localZ.cross(localX);

        float alpha = segmentArc;
        Vector3 A = a + (localY * radius);
        for (int i = 0; i < segments; ++i)
        {
            Vector3 offsetX = localX * (sinf(alpha) * radius);
            Vector3 offsetY = localY * (cosf(alpha) * radius);

            Vector3 B = a + offsetX + offsetY;
            Line(A, B, lineWidth, color);
            Line(A, A+ab, lineWidth, color);
            Line(A+ab, B+ab, lineWidth, color);
            A = B;
            alpha += segmentArc;
        }
    }

    void GLDraw3D::Sphere(Vector3 center, float radius, Color color)
    {
        // adaptive line count
        const int segments = 6 + (int(radius) / 6);
        const int rings   = segments;
        const int sectors = segments;

        const int totalVerts   = rings * sectors;
        const int totalIndices = totalVerts * 6;

        size_t nVerts = vertices.size();
        vertices.resize(nVerts + totalVerts);
        Vertex3Color* pVerts = &vertices[nVerts];

        indices.reserve(indices.size() + totalIndices);

        auto pushIndices = [&](int r, int s)
        {
            int curRow  = r * sectors;
            int nextRow = (r+1) * sectors;
            int nextS   = (s+1) % sectors;

            size_t nIndices = indices.size();
            indices.resize(nIndices + 6);
            index_t* ind = &indices[nIndices];
            ind[0] = curRow + s;
            ind[1] = nextRow + s;
            ind[2] = nextRow + nextS;

            ind[3] = curRow + s;
            ind[4] = nextRow + nextS;
            ind[5] = curRow + nextS;
        };

        constexpr float PI = rpp::PIf;
        constexpr float PI_2 = rpp::PIf / 2;
        const float R = 1./(float)(rings-1);
        const float S = 1./(float)(sectors-1);

        int vertexId = 0;
        for (int r = 0; r < rings; ++r)
        {
            for (int s = 0; s < sectors; ++s)
            {
                Vector3 pos {
                    center.x + radius * cos(2*PI * s * S) * sin( PI * r * R ),
                    center.y + radius * sin( -PI_2 + PI * r * R ),
                    center.z + radius * sin(2*PI * s * S) * sin( PI * r * R )
                };

                pVerts[vertexId++].set(pos, color);
                if (r < rings-1) {
                    pushIndices(r, s);
                }
            }
        }
    }

    void GLDraw3D::HollowSphere(Vector3 center, float radius, Color color, float lineWidth)
    {
        // adaptive line count
        const int segments = 8 + (int(radius) / 6);
        const int rings   = segments;
        const int sectors = segments;

        const int totalVerts = rings * sectors;
        ReserveLines(totalVerts);

        constexpr float PI = rpp::PIf;
        constexpr float PI_2 = rpp::PIf / 2;
        const float R = 1./(float)(rings-1);
        const float S = 1./(float)(sectors-1);

        auto spherePoint = [&](int r, int s) {
            return Vector3{
                center.x + radius * cos(2*PI * s * S) * sin( PI * r * R ),
                center.y + radius * sin( -PI_2 + PI * r * R ),
                center.z + radius * sin(2*PI * s * S) * sin( PI * r * R )
            };
        };

        Vector3 A = spherePoint(0, 0);

        for (int r = 0; r < rings; ++r)
        {
            for (int s = 0; s < sectors; ++s)
            {
                Vector3 B = spherePoint(r, s);
                Line(A, B, lineWidth, color);
                A = B;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
}

