#include"Grass.hlsli"

static const int THREAD_PER_NUM = 10;

struct ConstData
{
    float m_timeScale;
};

cbuffer cbuff0 : register(b0)
{
    ConstData g_constData;
}
RWStructuredBuffer<Grass> g_grassArray : register(u0);

//初期化処理
[numthreads(THREAD_PER_NUM, 1, 1)]
void InitMain(uint3 DTid : SV_DispatchThreadID)
{
    //草取得
    Grass grass = g_grassArray[DTid.x];

    //初期化処理==================
    
    //=========================

    //更新後の草適用
    g_grassArray[DTid.x] = grass;
}

//更新処理
[numthreads(THREAD_PER_NUM, 1, 1)]
void UpdateMain(uint3 DTid : SV_DispatchThreadID)
{
    //草取得
    Grass grass = g_grassArray[DTid.x];

    //更新処理==================
    
    //=========================

    //更新後の草適用
    g_grassArray[DTid.x] = grass;
}