#include"Camera.hlsli"

cbuffer cbuff0 : register(b0)
{
    matrix worldMat;
}

cbuffer cbuff1 : register(b1)
{
    Camera cam;
}

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

struct VSOutput
{
    int2 mirror : MIRROR;
    float alpha : ALPHA;
};

VSOutput VSmain(VSOutput input)
{
    return input;
}

struct GSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float alpha : ALPHA;
};

static const float DEFAULT_SIZE = 1.0f;
static const float DEFAULT_SIZE_HALF = DEFAULT_SIZE * 0.5f;

float4 AffectTransform(float4 pos)
{
    return mul(cam.proj, mul(cam.view, mul(worldMat, pos)));
}

[maxvertexcount(4)]
void GSmain(
    point VSOutput input[1],
    inout TriangleStream<GSOutput>output
)
{
    GSOutput element;
    element.alpha = input[0].alpha;
    
    //ç∂â∫
    element.pos = float4(-DEFAULT_SIZE_HALF, -DEFAULT_SIZE_HALF, 0.0f, 1.0f);
    element.pos = AffectTransform(element.pos);
    element.uv = float2(0.0f + input[0].mirror.x, 1.0f - input[0].mirror.y);
    output.Append(element);
    
    //ç∂è„
    element.pos = float4(-DEFAULT_SIZE_HALF, DEFAULT_SIZE_HALF, 0.0f, 1.0f);
    element.pos = AffectTransform(element.pos);
    element.uv = float2(0.0f + input[0].mirror.x, 0.0f + input[0].mirror.y);
    output.Append(element);
    
    //âEâ∫
    element.pos = float4(DEFAULT_SIZE_HALF, -DEFAULT_SIZE_HALF, 0.0f, 1.0f);
    element.pos = AffectTransform(element.pos);
    element.uv = float2(1.0f - input[0].mirror.x, 1.0f - input[0].mirror.y);
    output.Append(element);
    
    //âEè„
    element.pos = float4(DEFAULT_SIZE_HALF, DEFAULT_SIZE_HALF, 0.0f, 1.0f);
    element.pos = AffectTransform(element.pos);
    element.uv = float2(1.0f - input[0].mirror.x, 0.0f + input[0].mirror.y);
    output.Append(element);
}

float4 PSmain(GSOutput input) : SV_TARGET
{
    float4 col = tex.Sample(smp, input.uv);
    col.w *= input.alpha;
    return col;
}