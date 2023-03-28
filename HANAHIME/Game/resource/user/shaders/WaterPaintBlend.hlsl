#include"../../engine/ColorProcess.hlsli"
static const int THREAD_PER_NUM = 32;

Texture2D<float4> baseTex : register(t0);
Texture2D<float4> maskTex : register(t1);
RWTexture2D<float4> outputImage : register(u0);

[numthreads(THREAD_PER_NUM, THREAD_PER_NUM, 1)]
void CSmain(uint2 DTid : SV_DispatchThreadID)
{
    float4 base = baseTex[DTid];
    float4 blend = float4(ChangeHue(base.rgb, -141), base.w);
    float4 mask = maskTex[DTid];
    
    mask.x = saturate(mask.x);

    outputImage[DTid] = lerp(base, blend, 1.0f - mask.x);
};