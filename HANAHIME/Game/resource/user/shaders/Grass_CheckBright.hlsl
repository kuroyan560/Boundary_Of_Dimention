#include"../../engine/Camera.hlsli"
#include"Grass.hlsli"


RWStructuredBuffer<bool> checkBrightResultBuffer : register(u0);
StructuredBuffer<float3> grassPosArrayBuffer : register(t0);
Texture2D<float4> g_brightMap : register(t1);

cbuffer cbuff0 : register(b0)
{
    Camera cam;
}

cbuffer cbuff1 : register(b1)
{
    PlayerInfo player;
}

[numthreads(1, 1, 1)]
void CheckBright(uint DTid : SV_DispatchThreadID)
{
    //草の座標取得
    float3 grassPos = grassPosArrayBuffer[DTid];
    
    //現在位置にライトが当たっているかを確認する。
    float4 viewPos = mul(cam.view, float4(grassPos, 1.0f));
    float4 clipPos = mul(cam.proj, viewPos);
    float3 ndcPos = clipPos.xyz / clipPos.w;
    uint2 screenPos = round(float2((ndcPos.x * 0.5f + 0.5f) * 1280.0f, (1.0f - (ndcPos.y * 0.5f + 0.5f)) * 720.0f));
    float texColor = g_brightMap[screenPos].x;
    
    //プレイヤーとの距離
    float playerDistance = length(grassPos - player.m_pos);
    bool isNear = playerDistance <= player.m_plantLightRange;
  
    bool isBright = false;
    
    //光にあたっていたら草を生やし、 当たっていなかったらイージングタイマーを減らして草を枯らす。
    if (0.9f < texColor || isNear)
        isBright = true;
    
    //結果の格納
    checkBrightResultBuffer[DTid] = isBright;
};