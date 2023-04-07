#include"Math.hlsli"
struct VignetteInfo
{
    float4 m_color;
    float2 m_center;
    float m_intensity;
    float m_smoothness;
};

struct VSOutput
{
    float4 svpos : SV_POSITION; //システム用頂点座標
    float2 uv : TEXCOORD; //uv値
};

cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat; //平行投影行列
}

ConstantBuffer<VignetteInfo> vignetteInfo : register(b1);

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

VSOutput VSmain(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    VSOutput output; //ピクセルシェーダに渡す値
    output.svpos = mul(parallelProjMat, pos);
    pos.z = 0.0f;
    output.uv = uv;
    return output;
}

float4 PSmain(VSOutput input) : SV_TARGET
{
    // - ~ + 範囲に
    float2 uv = input.uv - vignetteInfo.m_center;
    
    float vignet = length(uv);
    
    float border = 1.0f - 0.5f * vignetteInfo.m_intensity;
    
    float rate = smoothstep(border - vignetteInfo.m_smoothness, border, vignet);
    
    return lerp(tex.Sample(smp, input.uv), vignetteInfo.m_color, rate);
}