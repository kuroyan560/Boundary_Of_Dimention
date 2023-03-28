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

static const int THREAD_PER_NUM = 32;
//static const int THREAD_PER_NUM = 1;

RWStructuredBuffer<MaskInk> aliveInkBuffer : register(u0);
ConsumeStructuredBuffer<MaskInk> consumeAliveInkBuffer : register(u0);
AppendStructuredBuffer<MaskInk> appendAliveInkBuffer : register(u0);

RWStructuredBuffer<uint> aliveInkCounterBuffer : register(u1);

StructuredBuffer<float3> stackInkPosBuffer : register(t0);

cbuffer cbuff0 : register(b0)
{
    ConstData constData;
}

cbuffer cbuff1 : register(b1)
{
    Camera cam;
}

[numthreads(1, 1, 1)]
void Init(uint DTid : SV_DispatchThreadID)
{
    consumeAliveInkBuffer.Consume();
};

[numthreads(1, 1, 1)]
void Appear(uint DTid : SV_DispatchThreadID)
{
    MaskInk newInk;
    newInk.m_pos = stackInkPosBuffer[DTid];
    newInk.m_scale = 2.0f;
    newInk.m_texIdx = 0;
    appendAliveInkBuffer.Append(newInk);
};

[numthreads(1, 1, 1)]
void Update(uint DTid : SV_DispatchThreadID)
{
};