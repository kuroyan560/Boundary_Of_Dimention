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
    float4 svpos : SV_POSITION; //�V�X�e���p���_���W
    float2 uv : TEXCOORD; //uv�l
};

cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat; //���s���e�s��
}

ConstantBuffer<VignetteInfo> vignetteInfo : register(b1);

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

VSOutput VSmain(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    VSOutput output; //�s�N�Z���V�F�[�_�ɓn���l
    output.svpos = mul(parallelProjMat, pos);
    pos.z = 0.0f;
    output.uv = uv;
    return output;
}

//3x3�̃K�E�V�A���t�B���^�������ăT���v�����O
float4 GaussianSample(float2 uv, float2 dx, float2 dy, float weight)
{
    float4 col = float4(0, 0, 0, 0);
    //����
    col += tex.Sample(smp, uv - dx - dy) * (weight / pow(2, 4)) / weight;
    //��
    col += tex.Sample(smp, uv - dx) * (weight / pow(2, 3)) / weight;
    //����
    col += tex.Sample(smp, uv - dx + dy) * (weight / pow(2, 4)) / weight;
    //��
    col += tex.Sample(smp, uv - dy) * (weight / pow(2, 3)) / weight;
    //�^��
    col += tex.Sample(smp, uv) * (weight / pow(2, 2)) / weight;
    //��
    col += tex.Sample(smp, uv + dy) * (weight / pow(2, 3)) / weight;
    //�E��
    col += tex.Sample(smp, uv + dx - dy) * (weight / pow(2, 4)) / weight;
    //�E
    col += tex.Sample(smp, uv + dx) * (weight / pow(2, 3)) / weight;
    //�E��
    col += tex.Sample(smp, uv + dx + dy) * (weight / pow(2, 4)) / weight;
    return col;
}

float4 PSmain(VSOutput input) : SV_TARGET
{
    // - ~ + �͈͂�
    float2 uv = input.uv - vignetteInfo.m_center;
    
    float vignet = length(uv);
    
    float border = 1.0f - 0.5f * vignetteInfo.m_intensity;
    
    float rate = smoothstep(border - vignetteInfo.m_smoothness, border, vignet);
    
    return lerp(tex.Sample(smp, input.uv), vignetteInfo.m_color, rate);
}