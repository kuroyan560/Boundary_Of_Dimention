#include"../../engine/Math.hlsli"
#include"../../engine/Camera.hlsli"

//判定結果
struct CheckResult
{
    float3 m_plantPos;
    int m_isSuccess;
    float3 m_plantNormal;
};

struct SearchPlantPosConstData
{
    int m_grassCount;
    float m_seed;
};

//ランダム
float RandomIntInRange(float arg_seed)
{
    return frac(sin(dot(float2(arg_seed, arg_seed + 1.0f), float2(12.9898f, 78.233f))) * 43758.5453f);
}

RWStructuredBuffer<CheckResult> searchPlantResultBuffer : register(u0);
StructuredBuffer<float3> grassPosArrayBuffer : register(t0);
Texture2D<float4> g_worldMap : register(t1);
Texture2D<float4> g_normalMap : register(t2);
Texture2D<float4> g_brightMap : register(t3);


cbuffer cbuff0 : register(b0)
{
    SearchPlantPosConstData constData;
}

[numthreads(16, 16, 1)]
void SearchPlantPos(uint3 GlobalID : SV_DispatchThreadID, uint3 GroupID : SV_GroupID, uint3 LocalID : SV_GroupThreadID)
{
    
    //グローバルIDの計算
    const int GRASS_SPAN = 10;
    uint index = GlobalID.y * (1280 / GRASS_SPAN) + GlobalID.x;
    
    //スクリーン座標からワールド座標へ変換。
    CheckResult result = searchPlantResultBuffer[index];
    
    //探す回数。
    //uint2 screenPos = (GroupID.xy * uint2(16, 16) + LocalID.xy) * uint2(GRASS_SPAN, GRASS_SPAN);
    uint2 screenPos = uint2(1280 / 2, 720 / 2);
    
    //ランダムで散らす。
    uint randomScatter = 200;
    uint2 random = uint2(RandomIntInRange(constData.m_seed * LocalID.x * GlobalID.y) * (randomScatter * 2), RandomIntInRange(constData.m_seed * LocalID.y * GlobalID.x) * (randomScatter * 2));
    random -= uint2(randomScatter, randomScatter);
    screenPos += random;
    
    //サンプリングした座標がライトに当たっている位置かどうかを判断。
    result.m_isSuccess = step(0.9f, g_brightMap[screenPos].x);
        
    //サンプリングに失敗したら次へ。
    if (result.m_isSuccess == 0)
    {
        searchPlantResultBuffer[index] = result;
        return;
    }
        
    //サンプリングに成功したらワールド座標を求める。
    result.m_plantPos = g_worldMap[screenPos].xyz;
        
    //法線も求める。
    result.m_plantNormal = g_normalMap[screenPos].xyz;
    
    //すでに生えているところにもう一度生やしていないかをチェック。
    for (int grassIndex = 0; grassIndex < constData.m_grassCount; ++grassIndex)
    {
        float distance = length(result.m_plantPos - grassPosArrayBuffer[grassIndex]);
        if (distance < 1.9f)
        {
            result.m_isSuccess = 0;
            break;
        }

    }
        
    searchPlantResultBuffer[index] = result;
    
}