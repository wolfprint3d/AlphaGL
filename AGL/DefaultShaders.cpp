#include "DefaultShaders.h"

namespace AGL
{
    ////////////////////////////////////////////////////////////////////////////

    ShaderPair GetEngineShaderSource(EngineShader engineShader)
    {
// LAZY init all the shaders
////////////////////////////////////////////////////////////////////////////////

static strview VS_PassthroughUVColor = R"END(
    // Basic passthrough 2D/3D vertex shader with UV coordinates and color
    uniform   highp mat4 transform; // transformation matrix
    attribute highp vec3 position;  // in vertex position (px,py,px)
    attribute highp vec2 coord;     // in vertex texture coordinates
    attribute highp vec4 color;     // rgba color

    varying highp vec2 vCoord;      // out vertex texture coord for frag
    varying highp vec4 vColor;

    void main(void)
    {
        gl_Position = transform * vec4(position, 1.0);
        vCoord = coord;
        vColor = color;
    }
)END";

////////////////////////////////////////////////////////////////////////////////

static strview PS_PassthroughColor = R"END(
    // Basic passthrough color-only shader
    uniform highp vec4 diffuseColor;

    void main(void)
    {
        highp vec4 texel = diffuseColor;
        if (texel.a < 0.025)
            discard;
        gl_FragColor = texel;
    }
)END";

static strview PS_VertexColor = R"END(
    // A simple vertex-color shader
    uniform highp vec4 diffuseColor;
    varying highp vec4 vColor;      // vertex color

    void main(void)
    {
        highp vec4 texel = vColor * diffuseColor;
        if (texel.a < 0.025)
            discard;
        gl_FragColor = texel;
    }
)END";

////////////////////////////////////////////////////////////////////////////////

static strview PS_TextureWithColor = R"END(
    // Basic texture + color shader
    // diffuseColor is used to multiply main color from diffuseTexture
    uniform highp sampler2D diffuseTex;   // diffuse texture
    uniform highp vec4      diffuseColor; // output color multiplier

    varying highp vec2 vCoord;    // vertex texture coords

    void main(void)
    {
        highp vec4 texel = texture2D(diffuseTex, vCoord) * diffuseColor;
        if (texel.a < 0.025)
            discard;
        gl_FragColor = texel;
    }
)END";

static strview PS_AlphaTextureWithColor = R"END(
    // Basic texture + color shader
    // diffuseColor is used to multiply main color from diffuseTexture
    uniform highp sampler2D diffuseTex;   // diffuse texture
    uniform highp vec4      diffuseColor; // output color multiplier

    varying highp vec2 vCoord;    // vertex texture coords

    void main(void)
    {
        // we assume diffuseTex is monochrome; -- reuse alpha from R channel
        highp float mainAlpha = texture2D(diffuseTex, vCoord).r;
        highp vec4 texel = mainAlpha * diffuseColor;
        if (texel.a < 0.025)
            discard;
        gl_FragColor = texel;
    }
)END";

////////////////////////////////////////////////////////////////////////////////

static strview PS_OutlineText = R"END(
    // Basic UI Text shader
    // diffuseColor is used to multiply main color from diffuseTexture
    uniform highp sampler2D diffuseTex;   // font atlas texture
    uniform highp vec4      diffuseColor; // font color
    uniform highp vec4      outlineColor; // font outline color

    varying highp vec2 vCoord;    // vertex texture coords

    void main(void)
    {
        // assume diffuseTex R is main color and G is outline
        highp vec2 tex = texture2D(diffuseTex, vCoord).rg;

        // color consists of the (diffuse color * main alpha) + (background color * outline alpha)
        highp vec3 color = (diffuseColor.rgb * tex.r) + (outlineColor.rgb * tex.g);
    
        // make the main alpha more pronounced, makes small text sharper
        tex.r = clamp(tex.r * 1.5, 0.0, 1.0);

        // alpha is the sum of main alpha and outline alpha
        // main alpha is main font color alpha
        // outline alpha is the stroke or shadow alpha
        highp float mainAlpha    = tex.r * diffuseColor.a;
        highp float outlineAlpha = tex.g * outlineColor.a * diffuseColor.a;
        gl_FragColor = vec4(color, mainAlpha + outlineAlpha);
    }
)END";

static strview PS_Rect = R"END(
    // Advanced distance-field shader for drawing rounded rects with border
    uniform highp vec4 diffuseColor; // output color multiplier
    uniform highp vec4 outlineColor; // border color

    // custom shader data:
    //    x: cornerRadius
    //    y: borderWidth
    //    z: absSize.x
    //    w: absSize.y
    uniform highp vec4 shaderData;

    varying highp vec2 vCoord;    // vertex texture coords

    highp float aa_step(highp float t1, highp float t2, highp float f)
    {
        //return step(t2, f);
        return smoothstep(t1, t2, f);
    }

    void main(void)
    {
        highp vec4 color = diffuseColor;
        if (color.a < 0.025)
            discard;

        gl_FragColor = color;
    }
)END";

        ////////////////////////////////////////////////////////////////////////////////

        static ShaderPair sources[] =
        {
            { strview{},                strview{}             }, // ES_None
            { PS_PassthroughColor,      VS_PassthroughUVColor }, // ES_Color
            { PS_PassthroughColor,      VS_PassthroughUVColor }, // ES_DebugLines
            { PS_PassthroughColor,      VS_PassthroughUVColor }, // ES_OutlineText
            { PS_Rect,                  VS_PassthroughUVColor }, // ES_Rect
            { PS_TextureWithColor,      VS_PassthroughUVColor }, // ES_Simple
            { PS_AlphaTextureWithColor, VS_PassthroughUVColor }, // ES_Text
            { PS_VertexColor,           VS_PassthroughUVColor }, // ES_VertexColor
            { PS_PassthroughColor,      VS_PassthroughUVColor }, // ES_Color3d
            { PS_TextureWithColor,      VS_PassthroughUVColor }, // ES_Simple3D
        };

        static_assert(sizeof(sources) == ES_Max*sizeof(ShaderPair), 
                        "Shader source array must match enum EngineShader layout");

        if (ES_None < engineShader && engineShader < ES_Max)
            return sources[engineShader];
        return ShaderPair{};
    }

    ShaderPair GetEngineShaderSource(strview name)
    {
        EngineShader shader = ES_None;
        if      (name == "color")       shader = ES_Color;
        else if (name == "debuglines")  shader = ES_DebugLines;
        else if (name == "outlinetext") shader = ES_OutlineText;
        else if (name == "rect")        shader = ES_Rect;
        else if (name == "simple")      shader = ES_Simple;
        else if (name == "text")        shader = ES_Text;
        else if (name == "vertexcolor") shader = ES_VertexColor;
        else if (name == "color3d")     shader = ES_Color3d;
        else if (name == "simple3d")    shader = ES_Simple3d;
        return GetEngineShaderSource(shader);
    }

    ////////////////////////////////////////////////////////////////////////////
}
