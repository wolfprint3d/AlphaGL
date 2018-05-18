//
// Created by Jorma on 25/05/17.
//
#pragma once
#include <rpp/strview.h>

namespace AGL
{
    using rpp::strview;
    ////////////////////////////////////////////////////////////////////////////////

    enum EngineShader
    {
        ES_None,
        ES_Color,       // color.frag       | color.vert        |
        ES_DebugLines,  // debuglines.frag  | debuglines.vert   |
        ES_OutlineText, // outlinetext.frag | outlinetext.vert  |
        ES_Rect,        // rect.frag        | rect.vert         |
        ES_Simple,      // simple.frag      | simple.vert       | A simple 2D UI shader !
        ES_Text,        // text.frag        | text.vert         |
        ES_VertexColor, // vertexcolor.frag | vertexcolor.vert  |
        ES_Color3d,     // color3d.frag     | color3d.vert      | A simple Vertex3DUV color only shader
        ES_Simple3d,    // simple3d.frag    | simple3d.vert     | A simple Vertex3DUV texture+color shader
        ES_Max,
    };

    struct ShaderPair
    {
        strview frag;
        strview vert;
        explicit operator bool() const { return (bool)frag && (bool)vert; }
    };

    ShaderPair GetEngineShaderSource(EngineShader engineShader);

    /**
     * Parsed values:
     *  "color"      - ES_Color
     *  "debuglines" - etc...
     *  "outlinetext"
     *  "rect"
     *  "simple"
     *  "text"
     *  "vertexalpha"
     *  "vertexcolor"
     *  "color3d"
     *  "simple3d"
     */
    ShaderPair GetEngineShaderSource(strview sourceName);

    ////////////////////////////////////////////////////////////////////////////////
}

