#include"../../engine/Camera.hlsli"

struct MaskInk
{
    float3 m_pos;
    float m_scale;
    int m_texIdx;
};

struct ConstData
{
    //座標ズレ最大
    float m_posOffsetMax;
    //インクテクスチャ数
    int m_texMax;
};

RWStructuredBuffer<MaskInk> aliveInkBuffer : register(u0);
StructuredBuffer<float3> stackInkPosBuffer : register(t0);

Texture2D<float4> inkTex_0 : register(t1);
Texture2D<float4> inkTex_1 : register(t2);
Texture2D<float4> inkTex_2 : register(t3);

cbuffer cbuff0 : register(b0)
{
    ConstData constData;
}

cbuffer cbuff1 : register(b1)
{
    Camera cam;
}

SamplerState smp : register(s0);

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
    int texIdx : TEX_IDX;
};

VSOutput VSmain(float3 pos : POSITION, float2 uv : TEXCOORD, uint instanceID : SV_InstanceID)
{
    MaskInk info = aliveInkBuffer[instanceID];
    float4 pos4 = float4(pos * info.m_scale, 1.0f);
    pos4 = mul(cam.billBoard, pos4);
    pos4.xyz += info.m_pos;
    
    VSOutput output;
    output.svpos = mul(cam.proj, mul(cam.view, pos4));
    output.uv = uv;
    output.texIdx = info.m_texIdx;
    return output;
}

float4 PSmain(VSOutput input) : SV_TARGET
{
    float4 result = float4(0, 0, 0, 0);
    
    if(input.texIdx == 0)
        result = inkTex_0.Sample(smp, input.uv);
    else if(input.texIdx == 1)
        result = inkTex_1.Sample(smp, input.uv);
    else if(input.texIdx == 2)
        result = inkTex_2.Sample(smp, input.uv);
    
    return result;
}