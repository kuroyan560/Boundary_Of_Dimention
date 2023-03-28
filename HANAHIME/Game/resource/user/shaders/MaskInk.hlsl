#include"../../engine/Camera.hlsli"
struct MaskInk
{
    float3 m_pos;
    float m_scale;
    float3 m_posOffset;
    int m_texIdx;
};

struct ConstData
{
    //マスクインクのスケール
    float m_initScale;
    //座標ズレ最大
    float m_posOffsetMax;
    //インクテクスチャ数
    int m_texMax;
};

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

//疑似乱数(-1.0f ~ 1.0f)
float GetRand(float arg_seed)
{
    float rand = frac(sin(arg_seed) * 143758.5453);
    //-1.0f ~ 1.0fに拡張
    rand = rand * 2.0f - 1.0f;
    return rand;
}
float3 GetRand(float3 arg_seed)
{
    return float3(GetRand(arg_seed.x), GetRand(arg_seed.y), GetRand(arg_seed.z));
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
    newInk.m_scale = constData.m_initScale;
    newInk.m_texIdx = 0;
    newInk.m_posOffset = GetRand(newInk.m_pos) * constData.m_posOffsetMax;
    appendAliveInkBuffer.Append(newInk);
};

[numthreads(1, 1, 1)]
void Update(uint DTid : SV_DispatchThreadID)
{
    //データ取得
    MaskInk ink = aliveInkBuffer[DTid];
  
    //次のテクスチャへ
    ink.m_texIdx = ink.m_texIdx + 1;
    //上限なら０に戻る
    if (constData.m_texMax <= ink.m_texIdx)
        ink.m_texIdx = 0;
    
    //座標オフセット更新
    ink.m_posOffset = GetRand(ink.m_posOffset) * constData.m_posOffsetMax;
    
    //更新されたデータを適用
    aliveInkBuffer[DTid] = ink;
};