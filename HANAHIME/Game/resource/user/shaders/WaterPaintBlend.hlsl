#include"../../engine/ColorProcess.hlsli"
Texture2D<float4> baseTex : register(t0);
Texture2D<int> maskTex : register(t1);
Texture2D<float> noiseTex : register(t2);
RWTexture2D<float4> outputImage : register(u0);

[numthreads(32, 32, 1)]
void CSmain(uint2 DTid : SV_DispatchThreadID)
{
    float4 base = baseTex[DTid];
    float4 blend = base;
    int mask = maskTex[DTid];
    if (1 - mask)
        blend = float4(ChangeHue(blend.rgb, -141), blend.w);
    float noise = noiseTex[DTid];
    
    //outputImage[DTid.xy] = float4(noise, noise, noise, 1.0f);
    outputImage[DTid.xy] = lerp(base, blend, 1 - mask);
};